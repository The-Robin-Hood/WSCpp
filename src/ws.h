#pragma once
#include <string>
#include <thread>
#include <vector>
#include <mutex>

#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/URI.h"

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPMessage;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::WebSocket;

enum class WebsocketState
{
    UNINITIALIZED,
    CONNECTING,
    CONNECTED,
    DISCONNECTED
};

struct Message
{
    enum class MessageType
    {
        SENT,
        RECEIVED
    };

    MessageType type;
    std::string content;
    std::chrono::system_clock::time_point timestamp;

    Message(MessageType type, const std::string &content) : type(type), content(content), timestamp(std::chrono::system_clock::now()) {}

    std::string getFormattedTimestamp() const
    {
        std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        return buffer;
    }
};

class WSC
{
public:
    WSC(const std::string &url);
    ~WSC();

    bool sendMsg(const std::string &message);
    void close();
    WebsocketState getState() const;
    std::vector<Message> getMessages();

private:
    WebsocketState _currentState;
    HTTPClientSession *_session;
    WebSocket *_websocket;

    std::vector<Message> *_messages;
    std::mutex _messagesMutex;

    std::thread _receiveThread;
    bool _running;

    std::string _scheme, _host, _path;
    int _port;
    bool _isSecure;

    void _receiveLoop();
    void parseURI(const std::string &url);
};