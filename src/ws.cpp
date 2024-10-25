#include "ws.h"
#include <string>
#include <memory>
#include <iostream>

// void listenForMessages(WebSocket *ws)
// {
//     char receiveBuff[256000];
//     int flags = 0;

//     try
//     {
//         while (running)
//         {
//             int rlen = ws->receiveFrame(receiveBuff, 256000, flags);
//             if (rlen <= 0)
//             {
//                 std::cout << "Connection closed by server." << std::endl;
//                 running = false; // Signal main thread to stop
//                 break;
//             }
//             receiveBuff[rlen] = '\0'; // Null-terminate received message
//             std::cout << "Received: " << receiveBuff << std::endl;
//         }
//     }
//     catch (std::exception &e)
//     {
//         std::cout << "Exception in listener thread: " << e.what() << std::endl;
//         running = false; // Signal main thread to stop
//     }
// }

// std::string hostExtraction(std::string uri)
// {
//     std::string host = uri;
//     if (host.find("://") != std::string::npos)
//     {
//         host = host.substr(host.find("://") + 3);
//     }
//     if (host.find(":") != std::string::npos)
//     {
//         host = host.substr(0, host.find(":"));
//     }
//     if (host.find("/") != std::string::npos)
//     {
//         host = host.substr(0, host.find("/"));
//     }
//     return host;
// }

// int portExtraction(std::string uri)
// {
//     int port = 80;
//     if (uri.find("://") != std::string::npos)
//     {
//         uri = uri.substr(uri.find("://") + 3);
//     }
//     if (uri.find(":") != std::string::npos)
//     {
//         port = std::stoi(uri.substr(uri.find(":") + 1));
//     }
//     return port;
// }

void ws::Init() noexcept
{
    try
    {
        ws::currentState = ws::ws_state::UNINITIALIZED;
        session.reset();
        websocket.reset();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Init error: " << e.what() << std::endl;
    }
}

bool ws::Connect(const std::string &url) noexcept
{
    try
    {
        if (currentState != ws::ws_state::UNINITIALIZED)
        {
            Close();
        }

        currentState = ws_state::CONNECTING;

        std::string scheme, host, path;
        int port;
        bool isSecure = false;

        try
        {
            // Remove ws:// or wss:// and add http:// or https:// for URI parsing
            std::string httpUrl = url;
            if (url.substr(0, 5) == "ws://")
            {
                httpUrl = "http://" + url.substr(5);
                isSecure = false;
            }
            else if (url.substr(0, 6) == "wss://")
            {
                httpUrl = "https://" + url.substr(6);
                isSecure = true;
            }
            else
            {
                // Invalid URL format
                throw std::runtime_error("Invalid WebSocket URL scheme. Must start with ws:// or wss://");
            }

            Poco::URI uri(httpUrl);
            scheme = uri.getScheme();
            host = uri.getHost();
            path = uri.getPathAndQuery();
            port = uri.getPort();

            // Use default ports if not specified
            if (port == -1)
            {
                port = isSecure ? 443 : 80;
            }

            if (path.empty())
            {
                path = "/";
            }
        }
        catch (const Poco::Exception &e)
        {
            throw std::runtime_error("Failed to parse URL: " + std::string(e.what()));
        }

        session = std::make_unique<HTTPClientSession>(host, port);

        HTTPRequest request(HTTPRequest::HTTP_GET, path,
                                       HTTPMessage::HTTP_1_1);
        request.set("User-Agent", "WSC++ v0.1");
        request.set("Upgrade", "websocket");
        request.set("Connection", "Upgrade");
        // request.set("Sec-WebSocket-Version", "13");
        // request.set("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ==");

        HTTPResponse response;

        websocket = std::make_unique<WebSocket>(*session, request, response);

        if (response.getStatus() == HTTPResponse::HTTP_SWITCHING_PROTOCOLS)
        {
            ws::currentState = ws::ws_state::CONNECTED;
            return true;
        }

        ws::currentState = ws::ws_state::DISCONNECTED;
        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Connection error: " << e.what() << std::endl;
        ws::currentState = ws::ws_state::DISCONNECTED;
        return false;
    }
}

bool ws::Send(const std::string &message) noexcept
{
    try
    {
        if (ws::currentState != ws::ws_state::CONNECTED || !ws::websocket)
        {
            return false;
        }

        int flags = WebSocket::FRAME_TEXT;
        int sent = ws::websocket->sendFrame(message.data(), message.size(), flags);

        return sent == message.size();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Send error: " << e.what() << std::endl;
        ws::currentState = ws::ws_state::DISCONNECTED;
        return false;
    }
}

bool ws::Receive() noexcept
{
    try
    {
        if (ws::currentState != ws::ws_state::CONNECTED || !ws::websocket)
        {
            return false;
        }

        const int BUFFER_SIZE = 1024;
        char buffer[BUFFER_SIZE];
        int flags = 0;

        int received = ws::websocket->receiveFrame(buffer, sizeof(buffer), flags);

        if (received > 0)
        {
            // Handle received data based on flags
            if (flags & WebSocket::FRAME_TEXT)
            {
                // Handle text frame
                std::string message(buffer, received);
                std::cout << "Received text: " << message << std::endl;
            }
            else if (flags & WebSocket::FRAME_BINARY)
            {
                // Handle binary frame
                std::cout << "Received binary data of size: " << received << std::endl;
            }
            return true;
        }

        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Receive error: " << e.what() << std::endl;
        ws::currentState = ws::ws_state::DISCONNECTED;
        return false;
    }
}

void ws::Close() noexcept
{
    try
    {
        if (ws::websocket)
        {
            ws::websocket->shutdown();
            ws::websocket.reset();
        }

        if (ws::session)
        {
            ws::session.reset();
        }

        ws::currentState = ws::ws_state::DISCONNECTED;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Close error: " << e.what() << std::endl;
        ws::currentState = ws::ws_state::DISCONNECTED;
    }
}
