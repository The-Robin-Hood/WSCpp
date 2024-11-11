
#include <iostream>
#include "ws.h"
#include <chrono>

int main()
{
    try
    {
        WSC::Config config;
        config.autoPing = false;
        config.subprotocols = {"lws-mirror-protocol"};
        WSC wsc("wss://libwebsockets.org/", config);
        if (!wsc.connect())
        {
            std::cout << "Failed to connect to server" << std::endl;
            return 1;
        }
        int i = 0;
        while (wsc.getCurrentState() != WSC::State::DISCONNECTED)
        {
            i++;
            std::this_thread::sleep_for(std::chrono::seconds(10));
            // std::string message = "HELLO WOORLD " + std::to_string(i);
            // if (!wsc.sendText(message))
            // {
            //     std::cerr << "Failed to send message" << std::endl;
            // }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}
