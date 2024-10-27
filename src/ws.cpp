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
        request.set("Sec-WebSocket-Version", "13");
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
        int total = 0;
        if (_currentState != WebsocketState::CONNECTED || !_websocket)
        {
            return false;
        }

        if (message.size() > 125)
        {
            std::string mutableMessage = message;
            bool first_frame = true;
            while (!mutableMessage.empty())
            {
                int flags = first_frame ? WebSocket::FRAME_OP_TEXT : WebSocket::FRAME_OP_CONT;
                int chunk_size = std::min(static_cast<int>(mutableMessage.size()), 125);
                if (chunk_size == mutableMessage.size())
                {
                    flags |= WebSocket::FRAME_FLAG_FIN;
                }
                int sent = _websocket->sendFrame(mutableMessage.data(), chunk_size, flags);
                mutableMessage = mutableMessage.substr(chunk_size);
                first_frame = false;
                total += sent;
                std::cout << "Sent " << sent << " bytes" << std::endl;
            }
        }
        else
        {
            int flags = WebSocket::FRAME_TEXT;
            int sent = _websocket->sendFrame(message.data(), message.size(), flags);
            total += sent;
        }

        {
            std::lock_guard<std::mutex> lock(_messagesMutex);
            _messages->push_back(Message(Message::MessageType::SENT, message));
        }

        return total == message.size();
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
    int contFrameCount = 0;

    while (_running)
    {
        try
        {
            Poco::Buffer<char> buffer(0);
            int flags;
            int n = _websocket->receiveFrame(buffer, flags);
            std::cout << "Received " << n << " bytes" << std::endl;

            int opcode = flags & WebSocket::FRAME_OP_BITMASK;
            switch (opcode)
            {
            case WebSocket::FRAME_OP_CONT:
            {
                std::cout << "FRAME_OP_CONT" << std::endl;
                if (n <= 0)
                {
                    contFrameCount++;
                    if (contFrameCount >= SERVER_CRASH_DETECTION_THRESHOLD)
                    {
                        std::cerr << "Server is not responding. Disconnecting." << std::endl;
                        _currentState = WebsocketState::DISCONNECTED;
                        _running = false;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Avoid tight loop
                    continue;
                }
                contFrameCount = 0;
                break;
            }
            case WebSocket::FRAME_OP_TEXT:
            {
                std::cout << "FRAME_OP_TEXT" << std::endl;
                std::string message(buffer.begin(), buffer.begin() + n);
                std::lock_guard<std::mutex> lock(_messagesMutex);
                _messages->push_back(Message(Message::MessageType::RECEIVED, message));
                break;
            }

            case WebSocket::FRAME_OP_BINARY:
            {
                std::cout << "FRAME_OP_BINARY" << std::endl;
                std::string message = "Binary data received (" + std::to_string(n) + " bytes)";
                std::lock_guard<std::mutex> lock(_messagesMutex);
                _messages->push_back(Message(Message::MessageType::RECEIVED, message));
                break;
            }

            case WebSocket::FRAME_OP_CLOSE:
            {
                std::cout << "FRAME_OP_CLOSE" << std::endl;
                std::cerr << "Received Close frame from server. Disconnecting." << std::endl;
                _currentState = WebsocketState::DISCONNECTED;
                _running = false;
                break;
            }

            case WebSocket::FRAME_OP_PING:
            {
                std::cout << "FRAME_OP_PING" << std::endl;
                std::cout << "Ping received" << std::endl;
                std::cout << std::string(buffer.begin(), buffer.begin() + n) << " " << n << std::endl;
                _websocket->sendFrame(buffer.begin(), n, WebSocket::FRAME_OP_PONG | WebSocket::FRAME_FLAG_FIN);
                std::cout << "Pong sent" << std::endl;
                break;
            }

            case WebSocket::FRAME_OP_PONG:
                std::cout << "FRAME_OP_PONG" << std::endl;
                break;
            default:
                std::cout << "Unknown opcode" << std::endl;
                break;
            }
        }
        catch (Poco::Exception &exc)
        {
            if (exc.code() == POCO_EAGAIN || exc.code() == POCO_ETIMEDOUT)
            {
                // std::cerr << "Timeout receiving message: " << exc.displayText() << std::endl;
                continue;
            }
            std::cerr << "Error receiving message: " << exc.displayText() << exc.code() << exc.name() << std::endl;
            _currentState = WebsocketState::DISCONNECTED;
            _running = false;
            break;
        }
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