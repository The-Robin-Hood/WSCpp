#include "../gui.h"

void GUI::connectionScreen(std::shared_ptr<websocketConnection> websocket) {
    if (websocket == nullptr) {
        WSCLog(error, "Websocket connection is null");
        return;
    }
    WSC *ws = websocket->ws ? websocket->ws.get() : nullptr;
    WSC::State currentWSState = ws ? ws->getCurrentState() : WSC::State::UNINITIALIZED;

    ImGui::Dummy(ImVec2(0.0f, 16.0f));
    ImGui::BeginDisabled(currentWSState == WSC::State::CONNECTED);
    WSCpp::UI::Component::Input({.label = "Connection URL",
                                 .hint = "wss://echowebsocket.org",
                                 .inputText = websocket->hostInput,
                                 .size = ImVec2(300.0f, 0.0f)});
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (WSCpp::UI::Component::Button(
            {.label = currentWSState == WSC::State::CONNECTED ? "Disconnect" : "Connect",
             .framePadding = ImVec2(12.0f, 8.0f),
             .variant = currentWSState == WSC::State::CONNECTED
                            ? WSCpp::UI::Component::variants::destructive
                            : WSCpp::UI::Component::variants::primary})) {
        if (currentWSState == WSC::State::CONNECTED) {
            try {
                ws->disconnect();
            } catch (...) {
                WSCLog(error, "Failed to disconnect websocket");
            }
        } else {
            WSC::Config config;
            try {
                if (ws == nullptr) {
                    ws = new WSC(websocket->hostInput, config);
                    websocket->ws = std::unique_ptr<WSC>(ws);
                    websocket->messages = std::make_unique<MessageQueue>();
                    ws->setControlMessageCallback([websocket](WSCMessage message) {
                        WSCLog(debug, "Control message: " + message.getPayload());
                        websocket->messages->push(message, true);
                    });
                    ws->setDataMessageCallback([websocket](WSCMessage message) {
                        message.type = WSCMessageType::RECEIVED;
                        WSCLog(debug, "Received message: " + message.getPayload());
                        websocket->messages->push(message, true);
                    });
                    ws->setStateChangeCallback([websocket](const std::string &state) {
                        std::string stateMessage = "State changed to " + state;
                        WSCLog(debug, stateMessage);
                        websocket->messages->push(
                            WSCMessage{WSCMessageType::RECEIVED,
                                       std::vector<unsigned char>(stateMessage.begin(),
                                                                  stateMessage.end())},
                            true);
                    });
                }
                if (!ws->connect()) {
                    WSCLog(error, "Failed to connect to " + websocket->hostInput);
                }
            } catch (const std::exception &e) {
                WSCLog(error, "Failed to connect to " + websocket->hostInput + ": " + e.what());
            }
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::Text("Messages:");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetCurrentWindow()->WindowPadding.x -
                         ImGui::CalcTextSize("Options Clear Copy").x - 35.0f);

    if (WSCpp::UI::Component::Button({.label = "Clear",
                                      .framePadding = ImVec2(4.0f, 4.0f),
                                      .variant = WSCpp::UI::Component::variants::ghost})) {
        if (websocket->messages) {
            websocket->messages->clear();
        }
    }
    ImGui::SameLine();
    if (WSCpp::UI::Component::Button({.label = "Copy",
                                      .framePadding = ImVec2(4.0f, 4.0f),
                                      .variant = WSCpp::UI::Component::variants::ghost})) {
        if (websocket->messages) {
            std::string messages;
            for (auto &message : websocket->messages->getVector()) {
                messages += message.getPayload() + "\n";
            }
            ImGui::SetClipboardText(messages.c_str());
        }
    }
    ImGui::SameLine();
    ImVec2 buttonPos = ImGui::GetCursorScreenPos();
    ImVec2 buttonSize = ImGui::CalcTextSize("Options");
    if (WSCpp::UI::Component::Button({.label = "Options",
                                      .framePadding = ImVec2(4.0f, 4.0f),
                                      .variant = WSCpp::UI::Component::variants::ghost})) {
        ImGui::OpenPopup("Options");
    }
    ImGui::SetNextWindowPos(ImVec2(buttonPos.x - 50.0f, buttonPos.y - 65.0f), ImGuiCond_Appearing);
    if (ImGui::BeginPopup("Options")) {
        WSCpp::UI::Component::CheckBox("AutoScroll", &websocket->autoScroll);
        WSCpp::UI::Component::CheckBox("Timestamp", &websocket->showTimeStamp);
        WSCpp::UI::Component::CheckBox("PingPong", &websocket->showPingPong);
        ImGui::EndPopup();
    }

    ImGui::BeginDisabled(ws == nullptr || ws->getCurrentState() != WSC::State::CONNECTED);
    ImGui::BeginChild("Messages", ImVec2(0, 200.0f), true);
    if (websocket->messages) {
        auto messages = websocket->messages->getVector();
        for (auto &message : messages) {
            std::string payload = message.getPayload();
            if(!websocket->showPingPong && (payload == "PING" || payload.starts_with("PONG"))) {
                continue;
            }
            std::string msg;
            if (websocket->showTimeStamp) {
                msg += "[" + message.getFormattedTimestamp() + "] ";
            }
            msg += message.messageTypeString() + ": ";
            msg += payload;
            ImGui::TextUnformatted(msg.c_str());
        }
        if (websocket->autoScroll) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    static bool focused = false;
    if (focused) {
        ImGui::SetKeyboardFocusHere();
        focused = false;
    }

    WSCpp::UI::Component::Input({
        .label = "Send Message",
        .hint = "Type your message here",
        .inputText = websocket->sendMsgInput,
        .size = ImVec2(ImGui::GetWindowWidth() - 100, ImGui::GetTextLineHeight() * 5),
        .multiline = true,
        .flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine,
    });
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(0.0f, 3.0f));
    if (WSCpp::UI::Component::Button({.label = "Send",
                                      .framePadding = ImVec2(20.0f, 8.0f),
                                      .variant = WSCpp::UI::Component::variants::secondary})) {
        WSCLog(info, "Send Button Pressed");
        if (websocket->sendMsgInput.length() > 0) {
            if (ws && ws->getCurrentState() == WSC::State::CONNECTED) {
                if (!ws->sendText(websocket->sendMsgInput)) {
                    WSCLog(error, "Failed to send message");
                }
                websocket->messages->push(
                    WSCMessage{WSCMessageType::SENT,
                               std::vector<unsigned char>(websocket->sendMsgInput.begin(),
                                                          websocket->sendMsgInput.end())},
                    true);
                websocket->sendMsgInput.clear();
            }
        }
        focused = true;
    }
    if (WSCpp::UI::Component::Button({
            .label = "Ping ",
            .framePadding = ImVec2(20.0f, 8.0f),
            .variant = WSCpp::UI::Component::variants::secondary,
        })) {
        WSCLog(info, "Ping Button Pressed");
        if (ws && ws->getCurrentState() == WSC::State::CONNECTED) {
            if (!ws->sendPing()) {
                WSCLog(error, "Failed to send ping");
            }
        }
    }
    ImGui::EndGroup();
    ImGui::EndDisabled();

    {  // Status bar
        const float statusBarHeight = 30.0f;
        ImGui::SetCursorPosY(ImGui::GetMainViewport()->Size.y - statusBarHeight);
        ImGui::Text("Status : ");
        ImGui::SameLine();
        if (currentWSState == WSC::State::UNINITIALIZED)
            ImGui::TextColored(RGBAtoIV4(200, 200, 200, 0.5f), "Uninitialized");
        else if (currentWSState == WSC::State::CONNECTING)
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connecting");
        else if (currentWSState == WSC::State::DISCONNECTING)
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Disconnecting");
        else if (currentWSState == WSC::State::DISCONNECTED ||
                 currentWSState == WSC::State::WS_ERROR)
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
        else if (currentWSState == WSC::State::CONNECTED)
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");

        ImGui::SameLine(ImGui::GetWindowWidth() - 80);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    }
}