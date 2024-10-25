#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/URI.h"

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::Net::WebSocket;

namespace ws{
    enum class ws_state
    {
        UNINITIALIZED,
        CONNECTING,
        CONNECTED,
        DISCONNECTED
    };

    static ws_state currentState;
    static std::unique_ptr<HTTPClientSession> session;
    static std::unique_ptr<WebSocket> websocket;

    void Init() noexcept;
    bool Connect(const std::string &url) noexcept;
    bool Send(const std::string &message) noexcept;
    bool Receive() noexcept;
    void Close() noexcept;
}