#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPClientSession.h"

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::Net::WebSocket;

std::atomic<bool> running(true);


namespace ws{
    enum class State
    {
        UNINITIALIZED,
        CONNECTING,
        CONNECTED,
        DISCONNECTED
    };

    void Init() noexcept;
    bool Connect(const std::string &host, const std::string &path) noexcept;
    bool Send(const std::string &message) noexcept;
    bool Receive() noexcept;
    void Close() noexcept;
}