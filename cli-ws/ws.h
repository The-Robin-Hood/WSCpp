#pragma once

#include <Poco/Buffer.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/PrivateKeyPassphraseHandler.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/URI.h>
#include <message.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SSLManager.h"

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPMessage;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPSClientSession;
using Poco::Net::WebSocket;

class MessageQueue;

class WSC {
   public:
    // Enums and Types
    enum class State {
        UNINITIALIZED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        DISCONNECTED,
        WS_ERROR
    };

    // Configuration structure
    struct Config {
        // Connection settings
        Poco::Timespan connectionTimeout;
        Poco::Timespan receiveTimeout;
        Poco::Timespan sendTimeout;
        std::chrono::milliseconds pingInterval;

        // Message settings
        int receiveMaxPayloadSize;
        int receiveBufferSize;
        int sendBufferSize;
        int sendChunkSize;

        // Retry settings
        bool autoReconnect;
        int maxRetryAttempts;
        std::chrono::seconds retryDelay;

        // SSL settings
        std::string certificatePath;
        std::string privateKeyPath;
        std::string caLocation;
        std::string cipherList;
        int verificationMode;

        // Additional settings
        std::map<std::string, std::string> customHeaders;
        std::vector<std::string> subprotocols;
        std::string userAgent;
        bool autoPing;
        bool autoPong;
        int pongThreshold;
        int serverCrashContinuationFrame;

        Config()
            : connectionTimeout(30, 0),                 // 30 seconds
              receiveTimeout(5, 0),                     // 5 seconds
              sendTimeout(5, 0),                        // 5 seconds
              pingInterval(5 * 1000),                   // 5 seconds
              receiveMaxPayloadSize(16 * 1024 * 1024),  // 16MB
              receiveBufferSize(64 * 1024),             // 64KB
              sendBufferSize(64 * 1024),                // 64KB
              sendChunkSize(4096),                      // 4KB
              autoReconnect(true),
              maxRetryAttempts(3),
              retryDelay(3),
              certificatePath(""),
              privateKeyPath(""),
              caLocation(""),
              cipherList("ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"),
              verificationMode(5),  // 0 - none, 1 - relaxed, 3 - strict, 5 - verifyOnce
              userAgent("WSC++ v1.0"),
              autoPing(true),
              pongThreshold(3),
              serverCrashContinuationFrame(10) {}
    };

    // Construction/Destruction
    explicit WSC(const std::string &url, const Config &config = Config{});
    ~WSC();

    // Delete copy/move constructors and assignment operators
    WSC(const WSC &) = delete;
    WSC &operator=(const WSC &) = delete;
    WSC(WSC &&) = delete;
    WSC &operator=(WSC &&) = delete;

    // Public interface
    bool connect();
    void disconnect(uint16_t code = 1000, const std::string &reason = "Normal closure");
    bool reconnect();

    // Sending methods
    bool sendPing();
    bool sendText(std::string &message);
    bool sendBinary(const std::vector<uint8_t> &data);

    // State management and information
    std::string stateToString(State state) const;
    inline State getCurrentState() const noexcept {
        return m_state.load(std::memory_order_acquire);
    }
    inline void setState(State state) { m_state.store(state, std::memory_order_release); }
    inline bool isConnected() const noexcept { return getCurrentState() == State::CONNECTED; }

    // Configuration
    void setConfig(const Config &config) noexcept { m_config = config; }
    const Config &getConfig() const noexcept { return m_config; }

    // Statistics and diagnostics
    struct Statistics {
        uint64_t messagesSent{0};
        uint64_t messagesReceived{0};
        uint64_t bytesSent{0};
        uint64_t bytesReceived{0};
        std::chrono::system_clock::time_point lastMessageTime;
        std::chrono::system_clock::time_point lastPingTime;
        std::chrono::system_clock::time_point lastPongTime;
        uint32_t reconnectAttempts{0};
    };

    Statistics getStatistics() const;

   private:
    // Internal state
    std::atomic<State> m_state = State::UNINITIALIZED;
    std::string m_url;
    std::string m_scheme;
    std::string m_host;
    std::string m_path;
    Config m_config;
    uint16_t m_port = 80;
    bool m_isSecure = false;
    int m_serverCrashContinuationFrame = 0;

    // State management
    void updateState(State newState);
    void updateStatistics(bool sent, size_t bytes);

    // Connection handling
    std::unique_ptr<Poco::Net::WebSocket> m_websocket;
    std::unique_ptr<Poco::Net::HTTPClientSession> m_session;
    std::unique_ptr<Poco::Net::HTTPSClientSession> m_secureSession;

    // Threading and its management
    bool m_sendThreadRunning = false;
    bool m_receiveThreadRunning = false;
    bool m_pingThreadRunning = false;
    int m_pongNotReceivedCount = 0;
    std::unique_ptr<std::thread> m_sendThread;
    std::unique_ptr<std::thread> m_receiveThread;
    std::unique_ptr<std::thread> m_pingThread;

    void startSendThread();
    void stopSendThread();
    void sendLoop();
    void startReceiveThread();
    void stopReceiveThread();
    void receiveLoop();
    void startPingThread();
    void stopPingThread();
    void pingLoop();

    // Message handling
    std::unique_ptr<MessageQueue> m_messageQueue;
    std::unique_ptr<Poco::Buffer<char>> m_receiveBuffer;

    // Processing frames
    bool processFrame(const Poco::Buffer<char> &buffer, size_t length, int flags);
    bool handleControlFrame(int opcode, const Poco::Buffer<char> &buffer, size_t length);
    bool handleDataFrame(int opcode, bool isFinal, const Poco::Buffer<char> &buffer,
                         size_t length);
    void handleClose(uint16_t code, const std::string &reason);

    // Statistics
    mutable std::mutex m_statsMutex;
    Statistics m_stats;

    // Utility methods
    void parseURI(const std::string &url);
    void stopThreads();
    void cleanupResources();
    bool sendFrame(const void *buffer, size_t length, int flags);
    int getOpcode(int flags) { return flags & MessageType::OPCODE_MASK; }
    bool isFinalFrame(int flags) { return (flags & MessageType::FIN) != 0; }
    bool isValidFrameLength(int opcode, size_t length);

    // Retry handling
    std::atomic<int> m_retryCount = 0;
    bool shouldRetry() const noexcept {
        return m_config.autoReconnect && m_retryCount < m_config.maxRetryAttempts;
    }
    void resetRetryCount() noexcept { m_retryCount = 0; }
    void incrementRetryCount() noexcept { m_retryCount++; }

    // Frame composition
    // struct FrameHeader {
    //     bool fin;
    //     bool rsv1;
    //     bool rsv2;
    //     bool rsv3;
    //     uint8_t opcode;
    //     bool mask;
    //     uint64_t payloadLength;
    //     uint32_t maskingKey;
    // };

    // bool composeFrame(FrameHeader& header, const void* payload, size_t length);
    // bool parseFrame(FrameHeader& header, const Buffer& buffer, size_t& offset);
};