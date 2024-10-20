#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPClientSession.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::Net::WebSocket;

std::atomic<bool> running(true);  // Flag to signal when the threads should stop

// Function for listening to the WebSocket server in a separate thread
void listenForMessages(WebSocket* ws) {
    char receiveBuff[256000];
    int flags = 0;

    try {
        while (running) {
            int rlen = ws->receiveFrame(receiveBuff, 256000, flags);
            if (rlen <= 0) {
                std::cout << "Connection closed by server." << std::endl;
                running = false;  // Signal main thread to stop
                break;
            }
            receiveBuff[rlen] = '\0';  // Null-terminate received message
            std::cout << "Received: " << receiveBuff << std::endl;
        }
    } catch (std::exception& e) {
        std::cout << "Exception in listener thread: " << e.what() << std::endl;
        running = false;  // Signal main thread to stop
    }
}

std::string hostExtraction(std::string uri) {
    std::string host = uri;
    if (host.find("://") != std::string::npos) {
        host = host.substr(host.find("://") + 3);
    }
    if (host.find(":") != std::string::npos) {
        host = host.substr(0, host.find(":"));
    }
    if (host.find("/") != std::string::npos) {
        host = host.substr(0, host.find("/"));
    }
    return host;
}

int portExtraction(std::string uri) {
    int port = 80;
    if (uri.find("://") != std::string::npos) {
        uri = uri.substr(uri.find("://") + 3);
    }
    if (uri.find(":") != std::string::npos) {
        port = std::stoi(uri.substr(uri.find(":") + 1));
    }
    return port;
}

int main(int argc, char** argv) {
    std::string uri;
    std::cout << "Enter WebSocket URI (e.g., ws://localhost:5000): ";
    std::cin >> uri;
    int port;

    if (uri.find("ws://") == std::string::npos) {
        std::cout << "Invalid URI. Please enter a WebSocket URI starting with 'ws://'" << std::endl;
        return 1;
    }

    std::string host = hostExtraction(uri);
    port = portExtraction(uri);

    std::cout<<"HOST : "<<host<< " PORT : "<<port<<std::endl;

    try {
        HTTPClientSession cs(host, port);
        HTTPRequest request(HTTPRequest::HTTP_GET, "/?encoding=text", HTTPMessage::HTTP_1_1);
        HTTPResponse response;

        WebSocket* m_psock = new WebSocket(cs, request, response);
        std::cout << "Connection established with server." << std::endl;

        // Start listening thread
        std::thread listenerThread(listenForMessages, m_psock);

        // Main thread for sending messages
        std::string userInput;
        while (running) {
            std::cout << "Enter message to send (type 'exit' to quit): ";
            std::getline(std::cin, userInput);

            if (userInput == "exit" || !running) {
                running = false;
                break;
            }

            int len = m_psock->sendFrame(userInput.c_str(), userInput.length(), WebSocket::FRAME_TEXT);
            std::cout << "Sent " << len << " bytes." << std::endl;
        }

        if (listenerThread.joinable()) {
            listenerThread.join();
        }

        m_psock->close();
        delete m_psock;

    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        running = false;
    }

    std::cout << "Quitting application gracefully..." << std::endl;
    return 0;
}
