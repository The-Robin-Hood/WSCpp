#include "ws.h"

WSC::WSC(const std::string &url)
{
    _currentState = WebsocketState::UNINITIALIZED;
    _messages = new std::vector<Message>();
    _running = true;

    try
    {
        parseURI(url);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Init error: " << e.what() << std::endl;
    }
    try
    {
        _currentState = WebsocketState::CONNECTING;
        _session = new HTTPClientSession(_host, _port);
        _session->setTimeout(Poco::Timespan(5, 0));
        HTTPRequest request(HTTPRequest::HTTP_GET, _path, HTTPMessage::HTTP_1_1);
        request.set("User-Agent", "WSC++ v0.1");
        request.set("Upgrade", "websocket");
        request.set("Connection", "Upgrade");
        HTTPResponse response;
        _websocket = new WebSocket(*_session, request, response);

        if (response.getStatus() == HTTPResponse::HTTP_SWITCHING_PROTOCOLS)
        {
            _currentState = WebsocketState::CONNECTED;
            std::cout << "Connected to WebSocket server" << std::endl;
            _receiveThread = std::thread(&WSC::_receiveLoop, this);
        }
        else
        {
            _currentState = WebsocketState::DISCONNECTED;
            throw std::runtime_error("Failed to connect to WebSocket server");
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Connection error: " << e.what() << std::endl;
        _currentState = WebsocketState::DISCONNECTED;
        throw e;
    }
}

WSC::~WSC()
{
    close();
}

void WSC::parseURI(const std::string &url)
{
    std::string httpUrl = url;
    try
    {
        // Remove ws:// or wss:// and add http:// or https:// for URI parsing
        if (url.substr(0, 5) == "ws://")
        {
            httpUrl = "http://" + url.substr(5);
            _isSecure = false;
        }
        else if (url.substr(0, 6) == "wss://")
        {
            httpUrl = "https://" + url.substr(6);
            _isSecure = true;
        }
        else
        {
            // Invalid URL format
            throw std::runtime_error("Invalid WebSocket URL scheme. Must start with ws:// or wss://");
        }

        Poco::URI uri(httpUrl);
        _scheme = uri.getScheme();
        _host = uri.getHost();
        _path = uri.getPathAndQuery();
        _port = uri.getPort();

        // Use default ports if not specified
        if (_port == -1)
        {
            _port = _isSecure ? 443 : 80;
            std::cout << "Using default port: " << _port << std::endl;
        }

        if (_path.empty())
        {
            _path = "/";
        }
    }
    catch (const Poco::Exception &e)
    {
        throw std::runtime_error("Failed to parse URL: " + std::string(e.what()));
    }
}

bool WSC::sendMsg(const std::string &message)
{
    try
    {
        if (_currentState != WebsocketState::CONNECTED || !_websocket)
        {
            return false;
        }

        int flags = WebSocket::FRAME_TEXT;
        int sent = _websocket->sendFrame(message.data(), message.size(), flags);

        {
            std::lock_guard<std::mutex> lock(_messagesMutex);
            _messages->push_back(Message(Message::MessageType::SENT, message));
        }

        return sent == message.size();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Send error: " << e.what() << std::endl;
        _currentState = WebsocketState::DISCONNECTED;
        return false;
    }
}

void WSC::_receiveLoop()
{
    int noDataCounter = 0;
    const int maxNoDataCount = 10;

    while (_running)
    {
        try
        {
            std::vector<char> buffer(1024);
            int flags;

            int n = _websocket->receiveFrame(buffer.data(), buffer.size(), flags);
            std::cout << "Flags: " << flags << std::endl;
            std::cout << "Received " << n << " bytes" << std::endl;

            // If no data is received, increment counter and check threshold
            if (n <= 0)
            {
                noDataCounter++;
                if (noDataCounter >= maxNoDataCount)
                {
                    std::cerr << "No data received for " << maxNoDataCount << " consecutive reads. Disconnecting." << std::endl;
                    _currentState = WebsocketState::DISCONNECTED;
                    _running = false;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Avoid tight loop
                continue;
            }

            noDataCounter = 0;

            // Check if it's a Close frame
            if (flags & WebSocket::FRAME_OP_CLOSE)
            {
                std::cerr << "Received Close frame from server. Disconnecting." << std::endl;
                _currentState = WebsocketState::DISCONNECTED;
                _running = false;
                break;
            }

            // Handle Text or Binary frames
            if (flags & WebSocket::FRAME_OP_TEXT || flags & WebSocket::FRAME_OP_BINARY)
            {
                // Check if buffer needs resizing
                if (n >= buffer.size())
                {
                    buffer.resize(n);                                                  // Resize buffer to the actual size received
                    n = _websocket->receiveFrame(buffer.data(), buffer.size(), flags); // Read again if resized
                }

                // Store the received message
                std::string message(buffer.data(), n);
                {
                    std::lock_guard<std::mutex> lock(_messagesMutex);
                    _messages->push_back(Message(Message::MessageType::RECEIVED, message));
                }
            }
        }
        catch (Poco::Exception &exc)
        {
            if (exc.code() == POCO_EAGAIN)
            {
                std::cerr << "Timeout receiving message: " << exc.displayText() << std::endl;
                continue;
            }
            std::cerr << "Error receiving message: " << exc.displayText() << exc.code() << exc.name() << std::endl;
            _currentState = WebsocketState::DISCONNECTED;
            _running = false;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void WSC::close()
{
    if (_websocket)
    {
        try
        {
            _websocket->shutdown(); // Forcefully shutdown connection
        }
        catch (Poco::Exception &exc)
        {
            std::cerr << "Error shutting down WebSocket: " << exc.displayText() << std::endl;
        }
    }

    if (_currentState == WebsocketState::CONNECTED)
    {
        _currentState = WebsocketState::DISCONNECTED;
        _running = false;
        if (_receiveThread.joinable())
        {
            _receiveThread.join();
        }
    }
    if (_websocket)
    {
        try
        {
            _websocket->close();
        }
        catch (Poco::Exception &exc)
        {
            std::cerr << "Error closing WebSocket: " << exc.displayText() << std::endl;
        }
        delete _websocket;
        _websocket = nullptr;
    }
    if (_session)
    {
        delete _session;
        _session = nullptr;
    }
}

WebsocketState WSC::getState() const
{
    return _currentState;
}

std::vector<Message> WSC::getMessages()
{
    std::lock_guard<std::mutex> lock(_messagesMutex);
    return *_messages;
}