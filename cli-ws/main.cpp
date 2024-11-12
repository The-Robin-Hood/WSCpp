
#include <chrono>
#include <iostream>

#include "ws.h"

int main() {
    try {
        WSC::Config config;
        // config.autoPing = false;
        config.subprotocols = {"echo"};
        WSC wsc("ws://192.168.0.175:9000", config);
        wsc.setDataMessageCallback([](const Message &msg) {
            std::cout << "[DataMessageCallback] [" << msg.getFormattedTimestamp() << "] ";
            std::cout << "Received message: ";
            std::cout << msg.getPayload() << std::endl;
        });
        wsc.setControlMessageCallback([](const Message &msg) {
            std::cout << "[ControlMessageCallback] [" << msg.getFormattedTimestamp() << "] ";
            std::cout << "Received control message: ";
            std::cout << msg.getPayload() << std::endl;
        });
        wsc.setStateChangeCallback([](const std::string &state) {
            std::cout << "[StateChangeCallback] State changed to: ";
            std::cout << state << std::endl;
        });
        if (!wsc.connect()) {
            std::cout << "Failed to connect to server" << std::endl;
            return 1;
        }
        int i = 0;
        while (wsc.getCurrentState() != WSC::State::DISCONNECTED) {
            i++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::string message = "HELLO WOORLD " + std::to_string(i);
            if (!wsc.sendText(message)) {
                std::cerr << "Failed to send message" << std::endl;
            }
            if (i == 50) {
                wsc.reconnect();
            }
            if (i > 100) {
                if (!wsc.sendText(std::string("close"))) {
                    std::cerr << "Failed to send message" << std::endl;
                }
            }
        }
        std::cout << "EXITING" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
