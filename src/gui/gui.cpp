#include "gui.h"

GUI::~GUI() {
    m_allMessages.reset();
    WSCLog(debug, "Destroying GUI");
}

bool GUI::init() {
    if (!glfwInit()) {
        WSCLog(error, "Failed to initialize GLFW");
        return false;
    }
    if (!initWindow()) {
        WSCLog(error, "Failed to initialize window");
        return false;
    }
    if (!initImGui()) {
        WSCLog(error, "Failed to initialize ImGui");
        return false;
    }
    return true;
}

bool GUI::setWindowIcon() {
    int width, height;
    if (!WSCpp::UI::loadTextureFromMemory(m_preLoadedlogo.data(), m_preLoadedlogo.size(),
                                          &m_logoTexture, &width, &height)) {
        WSCLog(error, "Failed to load texture from memory");
        return false;
    }
    return true;
}

bool GUI::initWindow() {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, false);
    m_window.reset(
        glfwCreateWindow(m_config.width, m_config.height, m_config.title.c_str(), NULL, NULL));
    if (!m_window) {
        glfwTerminate();
        WSCLog(error, "Failed to create window");
        return false;
    }

    glfwMakeContextCurrent(getWindow());
    glfwSwapInterval(m_config.vsync);
    return true;
}

bool GUI::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (ImGui::GetCurrentContext() == nullptr) {
        WSCLog(error, "Failed to create ImGui context");
        return false;
    }

    ImGuiIO &m_io = ImGui::GetIO();
    m_io.IniFilename = nullptr;
    ImGui::StyleColorsClassic();

    if (!ImGui_ImplGlfw_InitForOpenGL(getWindow(), true)) {
        WSCLog(error, "Failed to initialize ImGui for OpenGL-GLFW");
        return false;
    }

#if defined(__APPLE__)
    ImGui_ImplOpenGL3_Init("#version 120");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif

    static ImFontConfig cfg;
    const ImWchar ranges[] = {
        WCHAR(0x0020),  WCHAR(0x00FF),   // Basic Latin (A-Z, a-z, numbers, punctuation)
        WCHAR(0x0100),  WCHAR(0x024F),   // Extended Latin
        WCHAR(0x1F300), WCHAR(0x1F5FF),  // Miscellaneous Symbols and Pictographs (Emojis)
        WCHAR(0x1F900), WCHAR(0x1F9FF),  // Supplemental Symbols and Pictographs (More emojis)
        WCHAR(0x2600),  WCHAR(0x26FF),   // Miscellaneous Symbols (e.g., ☀, ☁, ☂)
        WCHAR(0x1F600), WCHAR(0x1F64F),  // Emoticons (Smiley faces)
        WCHAR(0x1F680), WCHAR(0x1F6FF),  // Transport and Map Symbols (e.g., 🚗, 🚕)
        WCHAR(0x2700),  WCHAR(0x27BF),   // Dingbats (e.g., ✈, ✉)
        WCHAR(0x2190),  WCHAR(0x21FF),   // Arrow symbols
        WCHAR(0x2B00),  WCHAR(0x2BFF),   // Miscellaneous
        WCHAR(0)};

    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    cfg.FontDataOwnedByAtlas = false;
    for (int i = 0; i < m_fontNames.size(); i++) {
        for (auto &fontSize : m_fontSizes) {
#ifdef _WIN32
            ImFont *f = m_io.Fonts->AddFontFromMemoryTTF(
                (void *)m_preLoadedfonts[i].data(), static_cast<int>(m_preLoadedfonts[i].size()),
                static_cast<float>(fontSize), &cfg, ranges);
#else
            std::string fontPath = m_fontsPath + "/" + m_fontNames[i] + ".ttf";
            ImFont *f = m_io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize, &cfg, ranges);
#endif
            if (!f) {
                WSCLog(error, "Failed to load font: " + m_fontNames[i]);
                return false;
            }
            m_fonts[m_fontNames[i]][std::to_string(fontSize)] = f;
        }
    }

    if (!m_io.Fonts->Build()) {
        WSCLog(error, "Failed to build fonts");
        return false;
    }
    m_io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable | ImGuiConfigFlags_DockingEnable;
    WSCLog(debug, "Number of Fonts loaded : " +
                      std::to_string(m_io.Fonts->Fonts.Size / m_fontNames.size()) + " ( " +
                      std::to_string(m_fontSizes.size()) + " sizes each)");
    if (!setWindowIcon()) {
        WSCLog(error, "Failed to set window icon");
        return false;
    }
    return true;
}

void GUI::cleanupImGui() {
    WSCLog(debug, "Cleaning up ImGui");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool GUI::shouldClose() const { return glfwWindowShouldClose(getWindow()); }

void GUI::closeGUI() { cleanupImGui(); }

void GUI::update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::beginBaseLayout() {
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});
    ImGui::Begin("##BaseLayout", NULL,
                 ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
    ImGui::PopStyleColor();
    ImGui::PushFont(m_fonts["Inter-Regular"]["14"]);
}

void GUI::endBaseLayout() {
    ImGui::PopFont();
    ImGui::End();
}

void GUI::titleBar() {
    const float titlebarHeight = 60.0f;
    float titlebarVerticalOffset = 0.0f;
    const ImVec2 windowPadding = ImGui::GetCurrentWindow()->WindowPadding;

    ImGui::PushFont(m_fonts["Inter-Bold"]["16"]);
    auto *fgDrawList = ImGui::GetForegroundDrawList();
    const float logoHorizontalOffset = 35.0f + windowPadding.x;
    ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
    {
        fgDrawList->AddImage(
            m_logoTexture, ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y),
            ImVec2(ImGui::GetCursorScreenPos().x + 35.0f, ImGui::GetCursorScreenPos().y + 35.0f));
    }
    ImGui::SetCursorPos(ImVec2(logoHorizontalOffset, 0.0f));
    const ImRect menuBarRect = {ImGui::GetCursorPos(),
                                {ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x,
                                 ImGui::GetFrameHeightWithSpacing()}};
    ImGui::SetItemAllowOverlap();
    if (WSCpp::UI::beginMenubar(menuBarRect)) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                WSCLog(debug, "Exit clicked");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                WSCLog(debug, "About clicked");
            }
            ImGui::EndMenu();
        }
        WSCpp::UI::endMenubar();
    }

    ImGui::PopFont();
}

void GUI::render() {
    // Clear the background
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render main application window

    // renderMainWindow();
    beginBaseLayout();
    titleBar();
    // renderStatusBar();
    endBaseLayout();
    // renderTitleBar();
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(getWindow());
}

// void GUI::renderStatusBar() {
//     // Create status bar at the bottom of the screen
//     const float statusBarHeight = 30.0f;
//     ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x,
//                                    ImGui::GetMainViewport()->Pos.y +
//                                        ImGui::GetMainViewport()->Size.y - statusBarHeight),
//                             ImGuiCond_FirstUseEver);
//     ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x, statusBarHeight),
//                              ImGuiCond_FirstUseEver);
//     ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
//     ImGui::Begin("##statusbar", nullptr,
//                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
//                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
//                      ImGuiWindowFlags_NoMove);

//     WSC::State currentWSState =
//         m_websocket ? m_websocket->getCurrentState() : WSC::State::UNINITIALIZED;
//     ImGui::Text("Status : ");
//     ImGui::SameLine();
//     if (currentWSState == WSC::State::UNINITIALIZED)
//         ImGui::TextColored(RGBAtoIV4(200, 200, 200, 0.5f), "Uninitialized");
//     else if (currentWSState == WSC::State::CONNECTING)
//         ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connecting");
//     else if (currentWSState == WSC::State::DISCONNECTING)
//         ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Disconnecting");
//     else if (currentWSState == WSC::State::DISCONNECTED || currentWSState ==
//     WSC::State::WS_ERROR)
//         ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
//     else if (currentWSState == WSC::State::CONNECTED)
//         ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");

//     ImGui::SameLine(ImGui::GetWindowWidth() - 80);
//     ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

//     ImGui::End();
// }

void GUI::renderMainWindow() {
    WSC::State currentWSState =
        m_websocket ? m_websocket->getCurrentState() : WSC::State::UNINITIALIZED;

    ImGui::SetNextWindowSize(
        ImVec2(ImGui::GetMainViewport()->Size.x, ImGui::GetMainViewport()->Size.y - 29.0f),
        ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
    ImGui::Begin("WSC", NULL,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoTitleBar);

    ImGui::Spacing();
    ImGui::Text("Host:");
    ImGui::InputTextWithHint("##Host", m_config.defaultHostHint.c_str(), &m_hostInput);
    ImGui::SameLine();

    if (currentWSState == WSC::State::CONNECTED) {
        if (ImGui::Button("Disconnect")) {
            try {
                m_websocket->disconnect();
            } catch (...) {
                WSCLog(error, "Failed to disconnect websocket");
            }
        }
    } else {
        if (ImGui::Button("Connect")) {
            WSCLog(info, "Connect Button Pressed");
            try {
                WSC::Config config;
                WSCLog(debug, "Setting up websocket configuration");
                // m_allMessages->clear();
                if (m_websocket == nullptr) {
                    WSCLog(debug, "Connecting to " + m_hostInput);
                    m_websocket = std::make_unique<WSC>(m_hostInput, config);
                    m_allMessages = std::make_unique<MessageQueue>();
                    m_websocket->setDataMessageCallback([this](WSCMessage message) {
                        message.type = WSCMessageType::RECEIVED;
                        if (m_allMessages != nullptr) {
                            WSCLog(debug, "Received message: " + message.getPayload());
                            m_allMessages->push(message, true);
                        }
                    });
                    m_websocket->setStateChangeCallback([this](const std::string &state) {
                        std::string stateMessage = "State changed to " + state;
                        if (m_allMessages != nullptr) {
                            WSCLog(debug, "State changed to " + state);
                            m_allMessages->push(
                                WSCMessage{WSCMessageType::RECEIVED,
                                           std::vector<unsigned char>(stateMessage.begin(),
                                                                      stateMessage.end())},
                                true);
                        }
                    });
                }
                if (!m_websocket->connect()) {
                    WSCLog(error, "Failed to connect to " + m_hostInput);
                }
            } catch (const std::exception &e) {
                WSCLog(error, "Failed to connect to " + m_hostInput + ": " + e.what());
            }
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::BeginDisabled(m_websocket == nullptr ||
                         m_websocket->getCurrentState() != WSC::State::CONNECTED);
    ImGui::Text("Messages:");
    ImGui::BeginChild("Messages", ImVec2(0, m_config.height - 250.0f), true);
    if (m_websocket && m_websocket->getCurrentState() == WSC::State::CONNECTED) {
        auto messages = m_allMessages->getVector();
        for (const auto &message : messages) {
            std::string inputId = "m_messages";
            inputId.append(std::to_string(message.timestamp.time_since_epoch().count()));
            std::string input = (message.type == WSCMessageType::SENT ? "⬆ [" : "⬇ [") +
                                message.getFormattedTimestamp() + "] " + message.getPayload();
            int lineCount = (int)std::count(input.begin(), input.end(), '\n') + 1;
            char *buffer = (char *)malloc(input.size() + 1);
            strcpy(buffer, input.c_str());

            ImGui::PushID(inputId.c_str());
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, message.type == WSCMessageType::SENT
                                                     ? IM_COL32(255, 229, 163, 255)
                                                     : IM_COL32(255, 255, 255, 255));
            ImGui::InputTextMultiline(
                "", buffer, input.size() + 1,
                ImVec2(-FLT_MIN, (ImGui::GetTextLineHeight() * lineCount) + 7),
                ImGuiInputTextFlags_ReadOnly);
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopID();
            free(buffer);
        }
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();

    static bool focusOnInput = false;
    if (focusOnInput) {
        ImGui::SetKeyboardFocusHere();
        focusOnInput = false;
    }
    bool send = ImGui::InputTextMultiline(
        "##Send", &m_sendMessageInput,
        ImVec2(ImGui::GetWindowWidth() - 100, ImGui::GetTextLineHeight() * 5),
        ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if (ImGui::Button("Send") || send) {
        WSCLog(info, "Send Button Pressed");
        if (m_sendMessageInput.length() > 0) {
            if (m_websocket && m_websocket->getCurrentState() == WSC::State::CONNECTED) {
                if (!m_websocket->sendText(m_sendMessageInput)) {
                    WSCLog(error, "Failed to send message");
                }
                m_allMessages->push(
                    WSCMessage{WSCMessageType::SENT,
                               std::vector<unsigned char>(m_sendMessageInput.begin(),
                                                          m_sendMessageInput.end())},
                    true);
                m_sendMessageInput.clear();
            }
        }
        focusOnInput = true;
    }
    ImGui::EndDisabled();

    ImGui::End();
}