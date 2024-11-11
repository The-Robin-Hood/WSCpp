#include "ws.h"

WSC::WSC(const std::string &url, const Config &config)
    : m_url(url),
      m_config(config)
{
    m_messageQueue = std::make_unique<MessageQueue>();
    try
    {
        parseURI(url);
        if (m_isSecure)
        {
            Poco::Net::Context::Ptr sslContext = new Poco::Net::Context(
                Poco::Net::Context::TLS_CLIENT_USE,
                m_config.certificatePath,                                                     // No certificate file
                m_config.privateKeyPath,                                                      // No private key file
                m_config.caLocation,                                                          // No CA location
                static_cast<Poco::Net::Context::VerificationMode>(m_config.verificationMode), // No verification
                9,                                                                            // Verification depth
                false,                                                                        // Don't load default CAs
                m_config.cipherList                                                           // Cipher list
            );
            Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> certHandler = new Poco::Net::AcceptCertificateHandler(false);
            Poco::Net::SSLManager::instance().initializeClient(nullptr, certHandler, sslContext);
            m_secureSession = std::make_unique<Poco::Net::HTTPSClientSession>(m_host, m_port);
            m_secureSession->setTimeout(m_config.connectionTimeout);
        }
        else
        {
            m_session = std::make_unique<Poco::Net::HTTPClientSession>(m_host, m_port);
            m_session->setTimeout(m_config.connectionTimeout);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Init error: " << e.what() << std::endl;
    }
}

WSC::~WSC()
{
    stopThreads();
    cleanupResources();
    if (m_state == State::CONNECTED)
    {
        updateState(State::DISCONNECTED);
    }
    if (m_sendThread && m_sendThread->joinable())
    {
        std::cout << "Joining send thread" << std::endl;
        m_sendThread->join();
    }
    if (m_receiveThread && m_receiveThread->joinable())
    {
        std::cout << "Joining receive thread" << std::endl;
        m_receiveThread->join();
    }
    if (m_pingThread && m_pingThread->joinable())
    {
        std::cout << "Joining ping thread" << std::endl;
        m_pingThread->join();
    }
}

// ================================== PUBLIC METHODS ==================================
bool WSC::connect()
{
    if (m_state == State::CONNECTING || m_state == State::CONNECTED)
    {
        std::cerr << "Already connected or connecting" << std::endl;
        return false;
    }
    updateState(State::CONNECTING);
    HTTPRequest request(HTTPRequest::HTTP_GET, m_path, HTTPMessage::HTTP_1_1);
    request.set("User-Agent", m_config.userAgent);
    request.set("Upgrade", "websocket");
    request.set("Connection", "Upgrade");
    request.set("Sec-WebSocket-Version", "13");

    if (!m_config.subprotocols.empty())
    {
        std::string subprotocols = "";
        for (const auto &protocol : m_config.subprotocols)
        {
            if (protocol != m_config.subprotocols.back())
                subprotocols += protocol + ", ";
            else
                subprotocols += protocol;
        }
        request.set("Sec-WebSocket-Protocol", subprotocols);
    }

    HTTPResponse response;
    try
    {
        m_websocket = std::make_unique<Poco::Net::WebSocket>(
            m_isSecure ? *m_secureSession : *m_session, request, response);
        m_websocket->setSendTimeout(m_config.sendTimeout);
        m_websocket->setReceiveTimeout(m_config.receiveTimeout);
        m_websocket->setMaxPayloadSize(m_config.receiveMaxPayloadSize);
        m_websocket->setSendBufferSize(m_config.sendBufferSize);
        m_websocket->setReceiveBufferSize(m_config.receiveBufferSize);

        //  LATER: Add more options
        // m_websocket->setLinger - SO_LINGER used to close connection gracefully
        // m_websocket->setKeepAlive(true)  // Keep connection alive
        // m_websocket->setNoDelay(true);  // Disable Nagle's algorithm

        if (response.getStatus() == HTTPResponse::HTTP_SWITCHING_PROTOCOLS)
        {
            updateState(State::CONNECTED);
            std::cout << "Connected to WebSocket server" << std::endl;
            startPingThread();
            startSendThread();
            startReceiveThread();
            resetRetryCount();
            return true;
        }
        else
        {
            throw std::runtime_error("Failed to connect to WebSocket server");
        }
    }
    catch (const Poco::Exception &exc)
    {
        std::cerr << "Init error: " << exc.displayText() << std::endl;
        if (shouldRetry())
        {
            std::cerr << "Retrying connection" << std::endl;
            incrementRetryCount();
            std::this_thread::sleep_for(m_config.retryDelay);
            updateState(State::UNINITIALIZED);
            return connect();
        }
        updateState(State::WS_ERROR);
        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Init error: " << e.what() << std::endl;
        updateState(State::WS_ERROR);
        return false;
    }
}

void WSC::disconnect(uint16_t code, const std::string &reason)
{
    if (m_state == State::DISCONNECTED || m_state != State::CONNECTED)
        return;

    std::vector<unsigned char> payload;
    payload.reserve(2 + reason.size());

    payload.push_back(static_cast<unsigned char>((code >> 8) & 0xFF));
    payload.push_back(static_cast<unsigned char>(code & 0xFF));

    payload.insert(payload.end(), reason.begin(), reason.end());

    if (!sendFrame(payload.data(), payload.size(), MessageType::CLOSE))
    {
        std::cerr << "Failed to send close frame" << std::endl;
        return;
    }
    updateState(State::DISCONNECTING);
}

bool WSC::reconnect()
{
    if (m_state == State::CONNECTED)
        disconnect();
    return connect();
}

bool WSC::sendPing()
{
    if (m_state == State::CONNECTED)
    {
        if (!sendFrame(nullptr, 0, MessageType::PING))
        {
            std::cerr << "Failed to send PING" << std::endl;
            return false;
        };
        return true;
    }
    return false;
}

bool WSC::sendText(std::string &message)
{
    if (m_state != State::CONNECTED)
        return false;
    std::vector<unsigned char> payload(message.begin(), message.end());
    m_messageQueue->push(Message{MessageType::TEXT, payload});
    return true;
}

bool WSC::sendBinary(const std::vector<uint8_t> &data)
{
    if (m_state != State::CONNECTED)
        return false;
    m_messageQueue->push(Message{MessageType::BINARY, data});
    return true;
}

// ================================== PRIVATE METHODS =================================

// ================================= SEND THREAD =================================
void WSC::startSendThread()
{
    if (m_sendThreadRunning)
        return;
    if (m_sendThread && m_sendThread->joinable())
    {
        m_sendThread->join();
    }
    std::cout << "Starting send thread" << std::endl;
    m_sendThreadRunning = true;
    m_sendThread = std::make_unique<std::thread>(&WSC::sendLoop, this);
}

bool WSC::sendFrame(const void *buffer, size_t length, int flags)
{
    if (m_state != State::CONNECTED)
        return false;
    try
    {
        int totalBytesSent = 0;
        if (length > m_config.sendChunkSize)
        {
            bool firstFrame = true;
            while (totalBytesSent < length)
            {
                if (!firstFrame)
                {
                    flags = MessageType::CONTINUATION;
                }
                if (totalBytesSent + m_config.sendChunkSize >= length)
                {
                    flags |= MessageType::FIN;
                }
                int bytesToSend = std::min(m_config.sendChunkSize, length - totalBytesSent);
                int sent = m_websocket->sendFrame(static_cast<const char *>(buffer) + totalBytesSent, bytesToSend, flags);
                if (sent < 0)
                {
                    std::cerr << "Failed to send message" << std::endl;
                    return false;
                }
                firstFrame = false;
                totalBytesSent += sent;
            }
        }
        else
        {
            flags |= MessageType::FIN;
            totalBytesSent = m_websocket->sendFrame(buffer, length, flags);
        }

        return totalBytesSent == length;
    }
    catch (const Poco::Exception &e)
    {
        std::cerr << "Send error: " << e.what() << e.displayText() << std::endl;
        updateState(State::WS_ERROR);
        return false;
    }
}

void WSC::sendLoop()
{
    while (m_sendThreadRunning)
    {
        try
        {
            Message m_message;
            m_messageQueue->try_pop(m_message);
            if (m_message.type != MessageType::UNINITIALIZED && m_state == State::CONNECTED)
            {
                if (!sendFrame(m_message.payload.data(), m_message.payload.size(), m_message.type))
                {
                    std::cerr << "Failed to send message" << std::endl;
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Send error: " << e.what() << std::endl;
            updateState(State::WS_ERROR);
        }
    }
    std::cout << "Send thread stopped" << std::endl;
}

void WSC::stopSendThread()
{
    if (!m_sendThreadRunning)
        return;
    m_sendThreadRunning = false;
}

// ================================= RECEIVE THREAD =================================

void WSC::startReceiveThread()
{
    if (m_receiveThreadRunning)
        return;
    if (m_receiveThread && m_receiveThread->joinable())
    {
        m_receiveThread->join();
    }
    std::cout << "Starting receive thread" << std::endl;
    m_receiveThreadRunning = true;
    m_receiveThread = std::make_unique<std::thread>(&WSC::receiveLoop, this);
}

bool WSC::handleControlFrame(int opcode, const Poco::Buffer<char> &buffer, size_t length)
{
    switch (opcode)
    {
    case MessageType::PING:
    {
        std::cout << "PING RECEIVED" << std::endl;
        if (m_config.autoPong)
        {
            if (!sendFrame(buffer.begin(), length, MessageType::PONG))
            {
                std::cerr << "Failed to send PONG" << std::endl;
            }
        }
        return true;
    }

    case MessageType::PONG:
    {
        std::cout << "PONG RECEIVED" << std::endl;
        m_pongNotReceivedCount = 0;
        return true;
    }

    case MessageType::CLOSE:
    {
        std::cout << "CLOSE RECEIVED" << std::endl;
        if (length < 2)
        {
            std::cerr << "Received WebSocket CLOSE frame with length less than 2" << std::endl;
            return false;
        }
        handleClose(
            static_cast<uint16_t>(static_cast<unsigned char>(buffer[0]) << 8 |
                                  static_cast<unsigned char>(buffer[1])),
            length > 2 ? std::string(buffer.begin() + 2, buffer.end()) : "");
        return true;
    }

    default:
        std::cerr << "Unknown control frame type: " << opcode << std::endl;
        return false;
    }
}

bool WSC::handleDataFrame(int opcode, bool isFinal, const Poco::Buffer<char> &buffer, size_t length)
{
    const char *frameTypes[] = {"CONTINUATION", "TEXT", "BINARY"};
    if (opcode <= MessageType::BINARY)
    {
        if (opcode == MessageType::CONTINUATION)
        {
            if (m_serverCrashContinuationFrame > m_config.serverCrashContinuationFrame)
            {
                return false;
            }
            m_serverCrashContinuationFrame++;
            return true;
        }
        m_serverCrashContinuationFrame = 0;
        std::cout << frameTypes[opcode] << " RECEIVED"
                  << (isFinal ? " (FINAL)" : " (FRAGMENT)") << std::endl;
        std::cout << "Payload: " << std::string(buffer.begin(), buffer.end()) << std::endl;
        return true;
    }

    std::cerr << "Unknown data frame type: " << opcode << std::endl;
    return false;
}

void WSC::handleClose(uint16_t code, const std::string &reason)
{
    std::cout << "Closing connection: " << code << " - " << reason << std::endl;
    updateState(State::DISCONNECTING);
}

bool WSC::processFrame(const Poco::Buffer<char> &buffer, size_t length, int flags)
{
    const int opcode = getOpcode(flags);
    const bool isFinal = isFinalFrame(flags);

    if (!isValidFrameLength(opcode, length))
    {
        std::cerr << "Invalid frame length: opcode=" << opcode << " length=" << length << std::endl;
        return false;
    }

    std::cout << "Processing frame: "
              << "Flags=" << flags << " "
              << "Opcode=" << opcode << " "
              << "Final=" << (isFinal ? "true" : "false") << " "
              << "Length=" << length << std::endl;

    // Handle control frames (PING, PONG, CLOSE)
    if (opcode >= MessageType::CLOSE)
    {
        return handleControlFrame(opcode, buffer, length);
    }

    // Handle data frames (TEXT, BINARY, CONTINUATION)
    return handleDataFrame(opcode, isFinal, buffer, length);
}

void WSC::receiveLoop()
{
    int errorFrameCount = 0;
    while (m_receiveThreadRunning)
    {
        try
        {
            Poco::Buffer<char> buffer(0);
            int flags;
            int n = m_websocket->receiveFrame(buffer, flags);

            if (!processFrame(buffer, n, flags))
            {
                if (m_serverCrashContinuationFrame > m_config.serverCrashContinuationFrame)
                {
                    std::cerr << "Server crash detected. Disconnecting." << std::endl;
                    updateState(State::WS_ERROR);
                    break;
                }

                std::cerr << "Failed to process frame" << std::endl;
                errorFrameCount++;
                if (errorFrameCount > 10)
                {
                    std::cerr << "Too many error frames. Disconnecting." << std::endl;
                    updateState(State::WS_ERROR);
                    break;
                }
            }
            else
            {
                errorFrameCount = 0;
            }
        }
        catch (Poco::Exception &exc)
        {
            if (exc.code() == POCO_EAGAIN || exc.code() == POCO_ETIMEDOUT ||
                exc.displayText() == "Timeout")
            {
                // ignore timeout - caused by receive timeout
                continue;
            }
            std::cerr << "Receive error: " << exc.what() << std::endl;
            updateState(State::WS_ERROR);
        }
    }
    std::cout << "Recieve thread stopped" << std::endl;
}

void WSC::stopReceiveThread()
{
    if (!m_receiveThreadRunning)
        return;
    m_receiveThreadRunning = false;
}

// ================================= PING THREAD =================================

void WSC::startPingThread()
{
    if (m_pingThreadRunning || m_config.autoPing == false)
        return;
    if (m_pingThread && m_pingThread->joinable())
    {
        m_pingThread->join();
    }
    std::cout << "Starting ping thread" << std::endl;
    m_pingThreadRunning = true;
    m_pingThread = std::make_unique<std::thread>(&WSC::pingLoop, this);
}

void WSC::pingLoop()
{
    while (m_pingThreadRunning)
    {
        if (m_state == State::CONNECTED)
        {
            if (!sendPing())
            {
                updateState(State::WS_ERROR);
                break;
            }
            if (m_pongNotReceivedCount > m_config.pongThreshold)
            {
                std::cerr << "Pong not received. Disconnecting." << std::endl;
                updateState(State::WS_ERROR);
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(m_config.pingInterval));
    }
    std::cout << "Ping thread stopped" << std::endl;
}

void WSC::stopPingThread()
{
    if (!m_pingThreadRunning)
        return;
    m_pingThreadRunning = false;
}

// ================================== HELPER METHODS ==================================

bool WSC::isValidFrameLength(int opcode, size_t length)
{
    if (opcode == MessageType::TEXT || opcode == MessageType::BINARY)
    {
        return length > 0 || !isFinalFrame(opcode);
    }
    return true;
}

void WSC::parseURI(const std::string &url)
{
    std::string httpUrl = url;
    // Remove ws:// or wss:// and add http:// or https:// for URI parsing
    if (url.substr(0, 5) == "ws://")
    {
        httpUrl = "http://" + url.substr(5);
        m_isSecure = false;
    }
    else if (url.substr(0, 6) == "wss://")
    {
        httpUrl = "https://" + url.substr(6);
        m_isSecure = true;
    }
    else
    {
        std::cerr << "Invalid WebSocket URL scheme. Must start with ws:// or wss://" << std::endl;
        throw std::runtime_error("Invalid WebSocket URL scheme. Must start with ws:// or wss://");
    }

    try
    {
        Poco::URI uri(httpUrl);
        m_scheme = uri.getScheme();
        m_host = uri.getHost();
        m_path = uri.getPathAndQuery();
        m_port = uri.getPort();

        if (m_path.empty())
        {
            m_path = "/";
        }
    }
    catch (const Poco::Exception &e)
    {
        throw std::runtime_error("Failed to parse URL: " + std::string(e.what()));
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to parse URL: " + std::string(e.what()));
    }
}

void WSC::stopThreads()
{
    stopSendThread();
    stopReceiveThread();
    stopPingThread();
}

void WSC::cleanupResources()
{
    std::cout << "Cleaning up WebSocket resources" << std::endl;

    if (m_websocket)
    {
        try
        {
            if (m_state == State::CONNECTED)
            {
                m_websocket->shutdown();
                m_websocket->close();
            }
        }
        catch (Poco::Exception &exc)
        {
            std::cerr << "Error shutting down WebSocket: " << exc.displayText() << std::endl;
        }
        m_websocket.reset();
    }

    m_session.reset();
    m_secureSession.reset();
    m_messageQueue.reset();
}

std::string WSC::stateToString(State state) const
{
    switch (state)
    {
    case State::UNINITIALIZED:
        return "UNINITIALIZED";
    case State::CONNECTING:
        return "CONNECTING";
    case State::CONNECTED:
        return "CONNECTED";
    case State::DISCONNECTING:
        return "DISCONNECTING";
    case State::DISCONNECTED:
        return "DISCONNECTED";
    case State::WS_ERROR:
        return "WS_ERROR";
    default:
        return "UNKNOWN";
    }
}

void WSC::updateState(State newState)
{
    std::cout << "State changed: " << stateToString(getCurrentState()) << " -> " << stateToString(newState) << std::endl;
    if (getCurrentState() == newState)
    {
        return;
    }
    setState(newState);
    if (m_state == State::WS_ERROR || m_state == State::DISCONNECTING)
    {
        stopThreads();
        if (m_state == State::WS_ERROR)
        {
            cleanupResources();
        }
        if (m_state == State::DISCONNECTING)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        updateState(State::DISCONNECTED);
    }
}
