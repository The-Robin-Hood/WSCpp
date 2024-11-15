#include "ws.h"

WSC::WSC(const std::string &url, const Config &config) : m_url(url), m_config(config) {
    m_messageQueue = std::make_unique<MessageQueue>();
    if (url.empty()) {
        throw std::invalid_argument("Empty URL provided");
    }
    parseURI(url);

    if (m_host.empty()) {
        throw std::invalid_argument("Invalid host");
    }

    if (m_port <= 0 || m_port > 65535) {
        throw std::invalid_argument("Invalid port number");
    }
    if (m_isSecure) {
        Poco::Net::Context::Ptr sslContext =
            new Poco::Net::Context(Poco::Net::Context::TLS_CLIENT_USE,
                                   m_config.certificatePath,  // No certificate file
                                   m_config.privateKeyPath,   // No private key file
                                   m_config.caLocation,       // No CA location
                                   static_cast<Poco::Net::Context::VerificationMode>(
                                       m_config.verificationMode),  // No verification
                                   9,                               // Verification depth
                                   false,                           // Don't load default CAs
                                   m_config.cipherList              // Cipher list
            );
        Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> certHandler =
            new Poco::Net::AcceptCertificateHandler(false);
        Poco::Net::SSLManager::instance().initializeClient(nullptr, certHandler, sslContext);
        m_secureSession = std::make_unique<Poco::Net::HTTPSClientSession>(m_host, m_port);
        m_secureSession->setTimeout(m_config.connectionTimeout);
    } else {
        m_session = std::make_unique<Poco::Net::HTTPClientSession>(m_host, m_port);
        m_session->setTimeout(m_config.connectionTimeout);
    }
}

WSC::~WSC() {
    WSCLog(debug, "Destroying WSC");
    stopThreads();
    if (m_sendThread && m_sendThread->joinable()) {
        WSCLog(debug, "Joining send thread");
        m_sendThread->join();
    }
    if (m_receiveThread && m_receiveThread->joinable()) {
        WSCLog(debug, "Joining receive thread");
        m_receiveThread->join();
    }
    if (m_pingThread && m_pingThread->joinable()) {
        WSCLog(debug, "Joining ping thread");
        m_pingThread->join();
    }
    WSCLog(debug, "All threads of WSC joined");
    if (m_state == State::CONNECTED) {
        updateState(State::DISCONNECTED);
    }
    cleanupResources();
}

// ================================== PUBLIC METHODS ==================================
bool WSC::connect() {
    if (m_state == State::CONNECTING || m_state == State::CONNECTED) {
        WSCLog(error, "Already connected or connecting");
        return false;
    }
    updateState(State::CONNECTING);
    HTTPRequest request(HTTPRequest::HTTP_GET, m_path, HTTPMessage::HTTP_1_1);
    request.set("User-Agent", m_config.userAgent);
    request.set("Upgrade", "websocket");
    request.set("Connection", "Upgrade");
    request.set("Sec-WebSocket-Version", "13");

    if (!m_config.subprotocols.empty()) {
        std::string subprotocols = "";
        for (const auto &protocol : m_config.subprotocols) {
            if (protocol != m_config.subprotocols.back())
                subprotocols += protocol + ", ";
            else
                subprotocols += protocol;
        }
        request.set("Sec-WebSocket-Protocol", subprotocols);
    }

    HTTPResponse response;
    try {
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

        if (response.getStatus() == HTTPResponse::HTTP_SWITCHING_PROTOCOLS) {
            updateState(State::CONNECTED);
            startPingThread();
            startSendThread();
            startReceiveThread();
            resetRetryCount();
            return true;
        } else {
            throw std::runtime_error("Failed to connect to WebSocket server");
        }
    } catch (const Poco::Exception &exc) {
        WSCLog(error, exc.displayText());
        if (shouldRetry()) {
            WSCLog(warn, "Retrying connection to " + m_host);
            incrementRetryCount();
            std::this_thread::sleep_for(m_config.retryDelay);
            updateState(State::UNINITIALIZED);
            return connect();
        }
        updateState(State::WS_ERROR, exc.displayText());
        return false;
    } catch (const std::exception &e) {
        WSCLog(error, e.what());
        updateState(State::WS_ERROR, e.what());
        return false;
    }
}

void WSC::disconnect(uint16_t code, const std::string &reason) {
    if (m_state == State::DISCONNECTED || m_state != State::CONNECTED) return;

    std::vector<unsigned char> payload;
    payload.reserve(2 + reason.size());

    payload.push_back(static_cast<unsigned char>((code >> 8) & 0xFF));
    payload.push_back(static_cast<unsigned char>(code & 0xFF));

    payload.insert(payload.end(), reason.begin(), reason.end());

    if (!sendFrame(payload.data(), payload.size(), MessageType::CLOSE)) {
        WSCLog(error, "Failed to send CLOSE frame");
        return;
    }
}

bool WSC::reconnect() {
    if (m_state == State::CONNECTED) disconnect();
    return connect();
}

bool WSC::sendPing() {
    if (m_state == State::CONNECTED) {
        if (!sendFrame(nullptr, 0, MessageType::PING)) {
            WSCLog(error, "Failed to send PING frame");
            return false;
        };
        return true;
    }
    return false;
}

bool WSC::sendText(std::string &message) {
    if (m_state != State::CONNECTED) return false;
    std::vector<unsigned char> payload(message.begin(), message.end());
    m_messageQueue->push(Message{MessageType::TEXT, payload});
    return true;
}

bool WSC::sendBinary(const std::vector<uint8_t> &data) {
    if (m_state != State::CONNECTED) return false;
    m_messageQueue->push(Message{MessageType::BINARY, data});
    return true;
}

// ================================== PRIVATE METHODS =================================

// ================================= SEND THREAD =================================
void WSC::startSendThread() {
    if (m_sendThreadRunning) return;
    if (m_sendThread && m_sendThread->joinable()) {
        m_sendThread->join();
    }
    WSCLog(debug, "Starting send thread");
    m_sendThreadRunning = true;
    m_sendThread = std::make_unique<std::thread>(&WSC::sendLoop, this);
}

bool WSC::sendFrame(const void *buffer, size_t length, int flags) {
    if (m_state != State::CONNECTED) return false;
    int len = static_cast<int>(length);
    try {
        int totalBytesSent = 0;
        if (len > m_config.sendChunkSize) {
            bool firstFrame = true;
            while (totalBytesSent < len) {
                if (!firstFrame) {
                    flags = MessageType::CONTINUATION;
                }
                if (totalBytesSent + m_config.sendChunkSize >= len) {
                    flags |= MessageType::FIN;
                }
                int bytesToSend = std::min<int>(m_config.sendChunkSize, len - totalBytesSent);
                int sent = m_websocket->sendFrame(
                    static_cast<const char *>(buffer) + totalBytesSent, bytesToSend, flags);
                if (sent < 0) {
                    return false;
                }
                firstFrame = false;
                totalBytesSent += sent;
            }
        } else {
            flags |= MessageType::FIN;
            totalBytesSent = m_websocket->sendFrame(buffer, len, flags);
        }

        return totalBytesSent == len;
    } catch (const Poco::Exception &e) {
        WSCLog(error,e.displayText());
        updateState(State::WS_ERROR, e.displayText());
        return false;
    }
}

void WSC::sendLoop() {
    while (m_sendThreadRunning) {
        try {
            Message m_message;
            m_messageQueue->try_pop(m_message);
            if (m_message.type != MessageType::UNINITIALIZED && m_state == State::CONNECTED) {
                if (!sendFrame(m_message.payload.data(), m_message.payload.size(),
                               m_message.type)) {
                    WSCLog(error, "Failed to send message");
                }
            }
        } catch (const std::exception &e) {
            WSCLog(error, "Send thread error: " + std::string(e.what()));
            updateState(State::WS_ERROR, e.what());
        }
    }
    WSCLog(debug, "Send Thread Loop stopped");
}

void WSC::stopSendThread() {
    if (!m_sendThreadRunning) return;
    m_sendThreadRunning = false;
}

// ================================= RECEIVE THREAD =================================

void WSC::startReceiveThread() {
    if (m_receiveThreadRunning) return;
    if (m_receiveThread && m_receiveThread->joinable()) {
        m_receiveThread->join();
    }
    WSCLog(debug, "Starting receive thread");
    m_receiveThreadRunning = true;
    m_receiveThread = std::make_unique<std::thread>(&WSC::receiveLoop, this);
}

bool WSC::handleControlFrame(int opcode, const Poco::Buffer<char> &buffer, size_t length) {
    switch (opcode) {
        case MessageType::PING: {
            WSCLog(info, "PING Received");
            std::string payload = "PING " + std::string(buffer.begin(), buffer.end());
            if (m_controlMessageCallback) {
                m_controlMessageCallback(Message{
                    MessageType::PING, std::vector<uint8_t>(payload.begin(), payload.end())});
            }
            if (m_config.autoPong) {
                if (!sendFrame(buffer.begin(), length, MessageType::PONG)) {
                    WSCLog(error, "Failed to send PONG");
                    return false;
                }
                WSCLog(debug, "PONG sent");
            }
            return true;
        }

        case MessageType::PONG: {
            WSCLog(info, "PONG Received");
            std::string payload = "PONG " + std::string(buffer.begin(), buffer.end());
            if (m_controlMessageCallback) {
                m_controlMessageCallback(Message{
                    MessageType::PONG, std::vector<uint8_t>(payload.begin(), payload.end())});
            }
            m_pongNotReceivedCount = 0;
            return true;
        }

        case MessageType::CLOSE: {
            WSCLog(info, "CLOSE Received");
            if (length < 2) {
                WSCLog(error, "Invalid CLOSE frame received");
                return false;
            }
            handleClose(static_cast<uint16_t>(static_cast<unsigned char>(buffer[0]) << 8 |
                                              static_cast<unsigned char>(buffer[1])),
                        length > 2 ? std::string(buffer.begin() + 2, buffer.end()) : "");
            return true;
        }

        default:
            WSCLog(warn, "Unknown control frame type: " + std::to_string(opcode));
            return false;
    }
}

bool WSC::handleDataFrame(int opcode, bool isFinal, const Poco::Buffer<char> &buffer,
                          size_t length) {
    const char *frameTypes[] = {"CONTINUATION", "TEXT", "BINARY"};
    if (!isFinal) {
        WSCLog(debug, "Recieving Continuation Frame");
    }
    if (opcode <= MessageType::BINARY) {
        if (opcode == MessageType::CONTINUATION) {
            if (m_serverCrashContinuationFrame > m_config.serverCrashContinuationFrame) {
                return false;
            }
            m_serverCrashContinuationFrame++;
            return true;
        }
        if (m_dataMessageCallback) {
            m_dataMessageCallback(Message{static_cast<MessageType>(opcode),
                                          std::vector<uint8_t>(buffer.begin(), buffer.end())});
        }
        m_serverCrashContinuationFrame = 0;
        return true;
    }

    WSCLog(warn, "Unknown data frame type: " + std::to_string(opcode) +
                     "length: " + std::to_string(length));
    return false;
}

void WSC::handleClose(uint16_t code, const std::string &reason) {
    if (reason.empty()) {
        WSCLog(debug,
               "Closing connection: " + std::to_string(code) + " - " + "No reason provided");
    } else {
        WSCLog(debug, "Closing connection: " + std::to_string(code) + " - " + reason);
    }
    std::string closePayload = std::to_string(code) + " - " + reason;
    if (m_controlMessageCallback) {
        m_controlMessageCallback(Message{
            MessageType::CLOSE, std::vector<uint8_t>(closePayload.begin(), closePayload.end())});
    }
    updateState(State::DISCONNECTING);
}

bool WSC::processFrame(const Poco::Buffer<char> &buffer, size_t length, int flags) {
    const int opcode = getOpcode(flags);
    const bool isFinal = isFinalFrame(flags);

    if (!isValidFrameLength(opcode, length)) {
        WSCLog(error, "Invalid frame length for opcode: " + std::to_string(opcode));
        return false;
    }

    WSCLog(debug,
           "Processing frame: "
           "Flags=" +
               std::to_string(flags) +
               " "
               "Opcode=" +
               std::to_string(opcode) +
               " "
               "Final=" +
               (isFinal ? "true" : "false") +
               " "
               "Length=" +
               std::to_string(length));

    // Handle control frames (PING, PONG, CLOSE)
    if (opcode >= MessageType::CLOSE) {
        return handleControlFrame(opcode, buffer, length);
    }

    // Handle data frames (TEXT, BINARY, CONTINUATION)
    return handleDataFrame(opcode, isFinal, buffer, length);
}

void WSC::receiveLoop() {
    int errorFrameCount = 0;
    while (m_receiveThreadRunning) {
        try {
            Poco::Buffer<char> buffer(0);
            int flags;
            int n = m_websocket->receiveFrame(buffer, flags);

            if (!processFrame(buffer, n, flags)) {
                if (m_serverCrashContinuationFrame > m_config.serverCrashContinuationFrame) {
                    WSCLog(warn, "Server crash detected");
                    updateState(State::WS_ERROR, "Server crash detected");
                    break;
                }
                errorFrameCount++;
                if (errorFrameCount > 10) {
                    WSCLog(warn, "Too many error frames received");
                    updateState(State::WS_ERROR, "Too many error frames");
                    break;
                }
            } else {
                errorFrameCount = 0;
            }
        } catch (Poco::Exception &exc) {
            if (exc.code() == POCO_EAGAIN || exc.code() == POCO_ETIMEDOUT ||
                exc.displayText() == "Timeout") {
                // ignore timeout - caused by receive timeout
                continue;
            }
            WSCLog(error, "Failed to receive frame: " + exc.displayText());
            updateState(State::WS_ERROR, exc.displayText());
        }
    }
    WSCLog(debug, "Receive Thread Loop stopped");
}

void WSC::stopReceiveThread() {
    if (!m_receiveThreadRunning) return;
    m_receiveThreadRunning = false;
}

// ================================= PING THREAD =================================

void WSC::startPingThread() {
    if (m_pingThreadRunning || m_config.autoPing == false) return;
    if (m_pingThread && m_pingThread->joinable()) {
        m_pingThread->join();
    }
    WSCLog(debug, "Starting ping thread");
    m_pingThreadRunning = true;
    m_pingThread = std::make_unique<std::thread>(&WSC::pingLoop, this);
}

void WSC::pingLoop() {
    while (m_pingThreadRunning) {
        if (m_state == State::CONNECTED) {
            if (!sendPing()) {
                updateState(State::WS_ERROR, "Failed to send PING");
                break;
            }
            if (m_pongNotReceivedCount > m_config.pongThreshold) {
                WSCLog(warn, "Pong not received for " + std::to_string(m_pongNotReceivedCount) +
                                 " times");
                updateState(State::WS_ERROR, "Pong not received");
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(m_config.pingInterval));
    }
    WSCLog(debug, "Ping Thread Loop stopped");
}

void WSC::stopPingThread() {
    if (!m_pingThreadRunning) return;
    m_pingThreadRunning = false;
}

// ================================== HELPER METHODS ==================================

bool WSC::isValidFrameLength(int opcode, size_t length) {
    if (opcode == MessageType::TEXT || opcode == MessageType::BINARY) {
        return length > 0 || !isFinalFrame(opcode);
    }
    return true;
}

void WSC::parseURI(const std::string &url) {
    std::string httpUrl = url;
    // Remove ws:// or wss:// and add http:// or https:// for URI parsing
    if (url.substr(0, 5) == "ws://") {
        httpUrl = "http://" + url.substr(5);
        m_isSecure = false;
    } else if (url.substr(0, 6) == "wss://") {
        httpUrl = "https://" + url.substr(6);
        m_isSecure = true;
    } else {
        throw std::invalid_argument("Invalid WebSocket URL scheme");
    }

    try {
        Poco::URI uri(httpUrl);
        m_scheme = uri.getScheme();
        m_host = uri.getHost();
        m_path = uri.getPathAndQuery();
        m_port = uri.getPort();

        if (m_path.empty()) {
            m_path = "/";
        }
    } catch (const Poco::Exception &e) {
        throw std::invalid_argument("Failed to parse URL: " + std::string(e.what()));
    } catch (const std::exception &e) {
        throw std::invalid_argument("Failed to parse URL: " + std::string(e.what()));
    }
}

void WSC::stopThreads() {
    stopSendThread();
    stopReceiveThread();
    stopPingThread();
}

void WSC::cleanupResources() {
    WSCLog(debug, "Cleaning up resources");
    if (m_websocket) {
        try {
            if (m_state == State::CONNECTED) {
                m_websocket->shutdown();
                m_websocket->close();
            }
        } catch (Poco::Exception &exc) {
            WSCLog(error, "Error shutting down WebSocket: " + exc.displayText());
        }
        m_websocket.reset();
    }

    m_session.reset();
    m_secureSession.reset();
    m_messageQueue.reset();
}

std::string WSC::stateToString(State state) const {
    switch (state) {
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

void WSC::updateState(State newState, std::string reason) {
    if (getCurrentState() == newState) {
        return;
    }
    WSCLog(info, "State changed: " + stateToString(getCurrentState()) + " -> " +
                     stateToString(newState));
    setState(newState);
    if (m_stateChangeCallback) {
        m_stateChangeCallback(stateToString(newState));
    }
    if (m_state == State::WS_ERROR || m_state == State::DISCONNECTING) {
        stopThreads();
        if (m_state == State::WS_ERROR && m_errorCallback) {
            m_errorCallback(reason);
        }
        cleanupResources();
        if (m_state == State::DISCONNECTING) {
            updateState(State::DISCONNECTED);
        }
    }
}
