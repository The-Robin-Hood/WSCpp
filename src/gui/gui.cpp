#include "gui.h"

#include "./resources/fonts.h"
#include "./resources/icons.h"
#include "./resources/logo.h"

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

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor) {
        WSCLog(error, "Failed to get primary monitor\n");
        glfwDestroyWindow(getWindow());
        glfwTerminate();
        return false;
    }
    const GLFWvidmode *videoMode = glfwGetVideoMode(monitor);
    int xpos = (videoMode->width - m_windowConfig.width) / 2;
    int ypos = (videoMode->height - m_windowConfig.height) / 3;
    glfwSetWindowPos(getWindow(), xpos, ypos);
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

    {
        auto closeWindowsIcon =
            WSCpp::UI::Resources::BinaryData(g_closeWindowsIcon_bin, g_closeWindowsIcon_size);
        auto minimizeWindowsIcon = WSCpp::UI::Resources::BinaryData(g_minimizeWindowsIcon_bin,
                                                                    g_minimizeWindowsIcon_size);
        auto defaultWindowsIcon =
            WSCpp::UI::Resources::BinaryData(g_defaultWindowsIcon_bin, g_defaultWindowsIcon_size);
        m_icons["closeWindowsIcon"] =
            std::make_shared<WSCpp::UI::Resources::Image>(closeWindowsIcon);
        m_icons["minimizeWindowsIcon"] =
            std::make_shared<WSCpp::UI::Resources::Image>(minimizeWindowsIcon);
        m_icons["defaultWindowsIcon"] =
            std::make_shared<WSCpp::UI::Resources::Image>(defaultWindowsIcon);
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

void GUI::events() {
    std::scoped_lock<std::mutex> lock(m_eventQueueMutex);
    while (m_eventQueue.size() > 0) {
        auto &func = m_eventQueue.front();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        func();
        m_eventQueue.pop();
    }
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
    const float logoWidth = 45.0f;
    const float logoHeight = 45.0f;
    const float logoHorizontalOffset = logoWidth + windowPadding.x + 10.0f;
    const ImVec2 titlebarMin = ImVec2(0.0f, 0.0f);
    const ImVec2 titlebarMax = {
        ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth() - windowPadding.y,
        ImGui::GetCursorScreenPos().y + titlebarHeight};

    ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
    {
        const ImVec2 logoOffset(windowPadding.x, windowPadding.y);
        const ImVec2 logoRectStart = {ImGui::GetItemRectMin().x + logoOffset.x,
                                      ImGui::GetItemRectMin().y + logoOffset.y};
        const ImVec2 logoRectMax = {logoRectStart.x + logoWidth, logoRectStart.y + logoHeight};
        ImVec4 dimColor = ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg];
        fgDrawList->AddImage(m_logoImage->getTexture(), logoRectStart, logoRectMax);
        if (ImGui::IsMouseHoveringRect(logoRectStart, logoRectMax)) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            // TODO: Add click event for logo
        }
        const bool dim_bg_for_modal = (ImGui::GetTopMostPopupModal() != NULL);
        if (dim_bg_for_modal) {
            fgDrawList->AddRectFilled(logoRectStart, logoRectMax,
                                      ImGui::GetColorU32(ImGuiCol_ModalWindowDimBg));
        }
    }

    const float buttonsAreaWidth = 40;
    {
        // Dragging the window with custom title bar
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 mousePos = ImGui::GetMousePos();

        static bool isDragging = false;
        static ImVec2 dragOffset;

        bool isMouseInDragZone = mousePos.x >= titlebarMin.x &&
                                 mousePos.x <= titlebarMax.x - buttonsAreaWidth &&
                                 mousePos.y >= titlebarMin.y && mousePos.y <= titlebarMax.y;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && isMouseInDragZone) {
            isDragging = true;
            dragOffset = ImVec2(mousePos.x - windowPos.x, mousePos.y - windowPos.y);
        }

        if (isDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            int newX = (int)(mousePos.x - dragOffset.x);
            int newY = (int)(mousePos.y - dragOffset.y);
            glfwSetWindowPos(getWindow(), newX, newY);
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            isDragging = false;
        }
    }
    ImGui::SetCursorPos(ImVec2(logoHorizontalOffset, 0.0f));
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

    //   Setting up the close and minimize buttons
    const float btnHW = 20.0f;
    const float icnHW = 16.0f;

    ImGui::SetCursorPos(
        ImVec2(ImGui::GetWindowWidth() - windowPadding.x - (icnHW * 2), windowPadding.y / 2));
    {
        if (ImGui::InvisibleButton("Minimize", ImVec2(btnHW, btnHW))) {
            m_eventQueue.push([window = getWindow()]() { glfwIconifyWindow(window); });
        }
        if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
            WSCpp::UI::Layout::DrawIcon(m_icons.at("minimizeWindowsIcon")->getTexture(), btnHW,
                                        btnHW, icnHW, icnHW);
        } else {
            WSCpp::UI::Layout::DrawIcon(m_icons.at("defaultWindowsIcon")->getTexture(), btnHW,
                                        btnHW, icnHW, icnHW);
        }
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10.0f);
    {
        if (ImGui::InvisibleButton("Close", ImVec2(btnHW, btnHW))) {
            glfwSetWindowShouldClose(getWindow(), GLFW_TRUE);
        }
        if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
            WSCpp::UI::Layout::DrawIcon(m_icons.at("closeWindowsIcon")->getTexture(), btnHW, btnHW,
                                        icnHW, icnHW);
        } else {
            WSCpp::UI::Layout::DrawIcon(m_icons.at("defaultWindowsIcon")->getTexture(), btnHW,
                                        btnHW, icnHW, icnHW);
        }
    }
    ImGui::PopFont();
    ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y + logoHeight));
}

void GUI::tabBar() {
    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        ImGui::ColorConvertU32ToFloat4(adjustTransparency(WSCpp::UI::Colors::primaryColor, 10)));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::ColorConvertU32ToFloat4(adjustTransparency(
                                                     WSCpp::UI::Colors::primaryColor, 10)));
    const ImVec2 windowPadding = ImGui::GetCurrentWindow()->WindowPadding;
    const float logoHorizontalOffset = 45.0f + windowPadding.x + 10.0f;
    ImGui::SetCursorPos(ImVec2(logoHorizontalOffset, ImGui::GetCursorPosY() + 5.0f));

    static ImVector<int> active_tabs;
    static int next_tab_id = 0;
    if (next_tab_id == 0) {
        active_tabs.push_back(next_tab_id++);
        active_tabs.push_back(next_tab_id++);
    }

    static ImGuiTabBarFlags tab_bar_flags =
        ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable |
        ImGuiTabBarFlags_FittingPolicyResizeDown | ImGuiTabBarFlags_FittingPolicyScroll;
    tab_bar_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyScroll);
    if (active_tabs.size() > 10) {
        tab_bar_flags |= ImGuiTabBarFlags_TabListPopupButton;
    } else {
        tab_bar_flags &= ~ImGuiTabBarFlags_TabListPopupButton;
    }
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
        // if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
        //     active_tabs.push_back(next_tab_id++);

        for (int n = 0; n < active_tabs.Size;) {
            bool open = true;
            char name[16];
            snprintf(name, IM_ARRAYSIZE(name), "%04d", active_tabs[n]);
            if (ImGui::BeginTabItem(name, &open, ImGuiTabItemFlags_None)) {
                ImGui::Text("This is the %s tab!", name);
                ImGui::EndTabItem();
            }

            if (!open)
                active_tabs.erase(active_tabs.Data + n);
            else
                n++;
        }
        ImGui::EndTabBar();
    }
    ImGui::PopStyleColor(2);
}

void GUI::render() {
    // Clear the background setting transparent color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    WSCpp::UI::Layout::beginBaseLayout(m_fonts["Inter-SemiBold"]["14"]);
    titleBar();
    // tabBar();

    if (m_websockets.find("default") == m_websockets.end()){
        m_websockets["default"] = std::make_shared<websocketConnection>();
    }
    connectionScreen(m_websockets["default"]);

    // ImGui::ShowDemoWindow();
    // WSCpp::UI::Component::showDemoUIComponents();
    WSCpp::UI::Layout::endBaseLayout();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(getWindow());
}
