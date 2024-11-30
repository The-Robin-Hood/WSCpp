#include "gui.h"

#include "./resources/resources.h"

GUI::~GUI() {
    m_logoImage.reset();
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

bool GUI::initWindow() {
    /*
    We are disabling window resizing, making the window transparent and removing the window border
    Since we are using custom title bar, we don't need the default window title bar
    */
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    m_window.reset(glfwCreateWindow(m_windowConfig.width, m_windowConfig.height,
                                    m_windowConfig.title.c_str(), NULL, NULL));
    if (!m_window) {
        glfwTerminate();
        WSCLog(error, "Failed to create window");
        return false;
    }
    glfwMakeContextCurrent(getWindow());
    glfwSwapInterval(m_windowConfig.vsync);
    return true;
}

bool GUI::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (ImGui::GetCurrentContext() == nullptr) {
        WSCLog(error, "Failed to create ImGui context");
        return false;
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(getWindow(), true)) {
        WSCLog(error, "Failed to initialize ImGui for OpenGL-GLFW");
        return false;
    }
    ImGui_ImplOpenGL3_Init("#version 150");

    ImGuiIO &m_io = ImGui::GetIO();
    m_io.IniFilename = nullptr;
    m_io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable | ImGuiConfigFlags_DockingEnable;

    {
        auto binaryData = WSCpp::UI::Resources::BinaryData(g_logo_bin, g_logo_size);
        m_logoImage = std::make_shared<WSCpp::UI::Resources::Image>(binaryData);
    }

    {
        std::vector<std::shared_ptr<WSCpp::UI::Resources::BinaryData>> preLoadedFonts = {
            std::make_shared<WSCpp::UI::Resources::BinaryData>(g_notoEmoji_bin, g_notoEmoji_size),
            std::make_shared<WSCpp::UI::Resources::BinaryData>(g_interRegular_bin,
                                                               g_interRegular_size),
            std::make_shared<WSCpp::UI::Resources::BinaryData>(g_interSemiBold_bin,
                                                               g_interSemiBold_size),
            std::make_shared<WSCpp::UI::Resources::BinaryData>(g_interBold_bin, g_interBold_size)};
        if (!WSCpp::UI::Resources::setupFonts(m_io, m_fonts, preLoadedFonts)) {
            WSCLog(error, "Failed while setting up fonts");
            return false;
        }
    }

    WSCpp::UI::Theme::setup();
    return true;
}

void GUI::closeGUI() {
    WSCLog(debug, "Cleaning up ImGui");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::titleBar() {
    const float titlebarHeight = 30.0f;
    const ImVec2 windowPadding = ImGui::GetCurrentWindow()->WindowPadding;

    ImGui::PushFont(m_fonts["Inter-Bold"]["16"]);
    auto *fgDrawList = ImGui::GetForegroundDrawList();
    const float logoHorizontalOffset = 35.0f + windowPadding.x;
    const ImVec2 titlebarMin = ImVec2(0.0f, 0.0f);
    const ImVec2 titlebarMax = {
        ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth() - windowPadding.y,
        ImGui::GetCursorScreenPos().y + titlebarHeight};

    ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
    {
        const float logoWidth = 40.0f;
        const float logoHeight = 40.0f;
        const ImVec2 logoOffset(windowPadding.x, windowPadding.y);
        const ImVec2 logoRectStart = {ImGui::GetItemRectMin().x + logoOffset.x,
                                      ImGui::GetItemRectMin().y + logoOffset.y};
        const ImVec2 logoRectMax = {logoRectStart.x + logoWidth, logoRectStart.y + logoHeight};
        fgDrawList->AddImage(m_logoImage->getTexture(), logoRectStart, logoRectMax);
    }

    const float buttonsAreaWidth = 50;
    {
        // Get the current window's position and the mouse position
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 mousePos = ImGui::GetMousePos();

        // Static variables to maintain dragging state across frames
        static bool isDragging = false;
        static ImVec2 dragOffset;

        // Check if the mouse is in the drag zone
        bool isMouseInDragZone = mousePos.x >= titlebarMin.x &&
                                 mousePos.x <= titlebarMax.x - buttonsAreaWidth &&
                                 mousePos.y >= titlebarMin.y && mousePos.y <= titlebarMax.y;

        // Start dragging
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && isMouseInDragZone) {
            isDragging = true;
            dragOffset = ImVec2(mousePos.x - windowPos.x, mousePos.y - windowPos.y);
        }

        // Continue dragging
        if (isDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            int newX = (int)(mousePos.x - dragOffset.x);
            int newY = (int)(mousePos.y - dragOffset.y);
            glfwSetWindowPos(getWindow(), newX, newY);
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            isDragging = false;
        }
    }
    ImGui::SetCursorPos(ImVec2(logoHorizontalOffset * 1.3, 0.0f));
    const ImRect menuBarRect = {
        ImGui::GetCursorPos(),
        {ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x - 30.0f,
         ImGui::GetFrameHeightWithSpacing()}};

    ImGui::BeginGroup();
    if (WSCpp::UI::Layout::beginMenubar(menuBarRect)) {
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
        WSCpp::UI::Layout::endMenubar();
    }
    ImGui::EndGroup();

    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 30.0f, 2.0f));
    if (ImGui::Button("X")) {
        glfwSetWindowShouldClose(getWindow(), true);
    }
    ImGui::PopFont();
}

void GUI::render() {
    // Clear the background setting transparent color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    WSCpp::UI::Layout::beginBaseLayout(m_fonts["Inter-SemiBold"]["14"]);
    titleBar();

    ImGui::SetCursorPosX(10.0f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);

    {
        if (WSCpp::UI::Component::Button({.label = "Primary Button"})) {
            WSCLog(debug, "Primary Button Clicked");
        }
        if (WSCpp::UI::Component::Button(
                {.label = "Destructive Button",
                 .variant = WSCpp::UI::Component::variants::destructive})) {
            WSCLog(debug, "Destructive Button Clicked");
        }
        if (WSCpp::UI::Component::Button({.label = "Secondary Button",
                                          .variant = WSCpp::UI::Component::variants::secondary})) {
            WSCLog(debug, "Secondary Button Clicked");
        }
        if (WSCpp::UI::Component::Button(
                {.label = "Outline Button", .variant = WSCpp::UI::Component::variants::outline})) {
            WSCLog(debug, "Outline Button Clicked");
        }
        if (WSCpp::UI::Component::Button(
                {.label = "Ghost Button", .variant = WSCpp::UI::Component::variants::ghost})) {
            WSCLog(debug, "Ghost Button Clicked");
        }
        if (WSCpp::UI::Component::Button({.label = "Disabed Button",
                                          .variant = WSCpp::UI::Component::variants::primary,
                                          .disabled = true})) {
            WSCLog(debug, "Disabled Button Clicked");
        }
    }

    {
        std::string normalText = "";
        WSCpp::UI::Component::Input({.label = "Normal Input",
                                     .hint = "Normal Input Field with hint text (default setting)",
                                     .inputText = normalText,
                                     .size = ImVec2(400.0f, 0.0f)});
    }

    // ImGui::ShowDemoWindow();
    // renderMainWindow();
    // renderStatusBar();
    WSCpp::UI::Layout::endBaseLayout();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(getWindow());
}

void GUI::renderStatusBar() {
    // Create status bar at the bottom of the screen
    const float statusBarHeight = 30.0f;
    ImGui::SetCursorPosY(ImGui::GetMainViewport()->Size.y - statusBarHeight);
    WSC::State currentWSState =
        m_websocket ? m_websocket->getCurrentState() : WSC::State::UNINITIALIZED;
    ImGui::Text("Status : ");
    ImGui::SameLine();
    if (currentWSState == WSC::State::UNINITIALIZED)
        ImGui::TextColored(RGBAtoIV4(200, 200, 200, 0.5f), "Uninitialized");
    else if (currentWSState == WSC::State::CONNECTING)
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connecting");
    else if (currentWSState == WSC::State::DISCONNECTING)
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Disconnecting");
    else if (currentWSState == WSC::State::DISCONNECTED || currentWSState == WSC::State::WS_ERROR)
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
    else if (currentWSState == WSC::State::CONNECTED)
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");

    ImGui::SameLine(ImGui::GetWindowWidth() - 80);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
}

void GUI::renderMainWindow() {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
    WSC::State currentWSState =
        m_websocket ? m_websocket->getCurrentState() : WSC::State::UNINITIALIZED;

    ImGui::Spacing();
    ImGui::Text("Host:");
    ImGui::InputTextWithHint("##Host", m_windowConfig.defaultHostHint.c_str(), &m_hostInput);
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
    ImGui::BeginChild("Messages", ImVec2(0, m_windowConfig.height - 250.0f), true);
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
}