#define STB_IMAGE_IMPLEMENTATION
#include "gui.h"

#include "imgui.h"
#include "imgui_custom.h"
#include "imgui_freetype.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "stb_image.h"

bool GUI::init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    if (!initWindow()) {
        std::cerr << "Failed to initialize window" << std::endl;
        return false;
    }
    if (!initImGui()) {
        std::cerr << "Failed to initialize ImGui" << std::endl;
        return false;
    }
    return true;
}

bool GUI::setWindowIcon() {
    int width, height, channels;
    unsigned char *image = stbi_load(m_config.logoPath.c_str(), &width, &height, &channels, 4);

    if (image) {
        // Create GLFW image structure
        GLFWimage icons[1];
        icons[0].width = width;
        icons[0].height = height;
        icons[0].pixels = image;
        glfwSetWindowIcon(getWindow(), 1, icons);
        stbi_image_free(image);
        return true;
    }
    return false;
}

bool GUI::initWindow() {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window.reset(
        glfwCreateWindow(m_config.width, m_config.height, m_config.title.c_str(), NULL, NULL));
    if (!m_window) {
        glfwTerminate();
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }
    if (!setWindowIcon()) {
        std::cerr << "Failed to set window icon" << std::endl;
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
        std::cerr << "Error: Failed to create ImGui context!" << std::endl;
        return false;
    }

    ImGuiIO &m_io = ImGui::GetIO();
    m_io.IniFilename = nullptr;
    ImGui::StyleColorsClassic();

    if (!ImGui_ImplGlfw_InitForOpenGL(getWindow(), true)) {
        std::cerr << "Failed to initialize ImGui for GLFW" << std::endl;
        return false;
    }

#if defined(__APPLE__)
    ImGui_ImplOpenGL3_Init("#version 120");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif

    m_io.Fonts->AddFontDefault();
    static ImWchar ranges[] = {0x1, 0xFFFF, 0};
    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.MergeMode = true;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    for (const auto &font : m_config.fonts) {
        std::string fontPath = "assets/fonts/"+font;
        if (!m_io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 13.0f, &cfg, ranges)) {
            std::cerr << "Error: Failed to load font: " << fontPath << std::endl;
            return false;
        }
    }
    if (!m_io.Fonts->Build()) {
        std::cerr << "Error: Failed to build fonts!" << std::endl;
        return false;
    }
    m_io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable | ImGuiConfigFlags_DockingEnable;
    return true;
}

void GUI::cleanupImGui() {
    std::cout << "Cleaning up ImGui" << std::endl;
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

void GUI::render() {
    // Clear the background
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render main application window
    renderMainWindow();
    renderStatusBar();

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
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x,
                                   ImGui::GetMainViewport()->Pos.y +
                                       ImGui::GetMainViewport()->Size.y - statusBarHeight),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x, statusBarHeight),
                             ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
    ImGui::Begin("##statusbar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoMove);

    ImGui::Text("Status: %s", "Connected");
    ImGui::SameLine(ImGui::GetWindowWidth() - 80);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    ImGui::End();
}

void GUI::renderMainWindow() {
    WebsocketState current_ws_state =
        m_websocket ? m_websocket->getState() : WebsocketState::UNINITIALIZED;

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

    if (current_ws_state == WebsocketState::CONNECTED) {
        if (ImGui::Button("Disconnect")) {
            try {
                m_websocket->close();
            } catch (...) {
                std::cerr << "Error closing WebSocket: " << std::endl;
            }
        }
    } else {
        if (ImGui::Button("Connect")) {
            std::cout << "Connecting to " << m_hostInput << std::endl;
            try {
                m_websocket = std::make_unique<WSC>(m_hostInput);
            } catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::Text("Status : ");
    ImGui::SameLine();
    if (current_ws_state == WebsocketState::UNINITIALIZED)
        ImGui::TextColored(RGBAtoIV4(200, 200, 200, 0.5f), "Uninitialized");
    else if (current_ws_state == WebsocketState::CONNECTING)
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connecting");
    else if (current_ws_state == WebsocketState::DISCONNECTED)
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
    else if (current_ws_state == WebsocketState::CONNECTED)
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");

    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    ImGui::BeginDisabled(m_websocket == nullptr ||
                         m_websocket->getState() != WebsocketState::CONNECTED);
    ImGui::Text("Messages:");
    ImGui::BeginChild("Messages", ImVec2(0, m_config.height - 250), true);
    if (m_websocket && m_websocket->getState() == WebsocketState::CONNECTED) {
        auto messages = m_websocket->getMessages();
        for (auto &message : messages) {
            std::string inputId = "m_messages";
            inputId.append(std::to_string(message.timestamp.time_since_epoch().count()));
            std::string input = (message.type == Message::MessageType::SENT ? "⬆ [" : "⬇ [") +
                                message.getFormattedTimestamp() + "] " + message.content;
            int lineCount = std::count(input.begin(), input.end(), '\n') + 1;
            char *buffer = (char *)malloc(input.size() + 1);
            strcpy(buffer, input.c_str());

            ImGui::PushID(inputId.c_str());
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, message.type == Message::MessageType::SENT
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
        std::cout << "Sending message: " << m_sendMessageInput << std::endl;
        if (m_sendMessageInput.length() > 0) {
            if (m_websocket && m_websocket->getState() == WebsocketState::CONNECTED) {
                std::cout << "Message sending" << std::endl;

                m_websocket->sendMsg(m_sendMessageInput);
                m_sendMessageInput.clear();
                std::cout << "Message sent" << std::endl;
            }
        }
        focusOnInput = true;
    }
    ImGui::EndDisabled();

    ImGui::End();
}