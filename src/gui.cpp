#include "gui.h"
#include <iostream>
#include <filesystem>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_custom.h"
#include "imgui_freetype.h"

#include "ws.h"

WSC *gui::ws = nullptr;

void gui::CreateGlfWindow(const char *title) noexcept
{
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, title, nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        std::cerr << "Failed to create window" << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
}

void gui::DestroyGlfWindow() noexcept
{
    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
}

void gui::CreateImGui() noexcept
{
    std::string path = std::filesystem::current_path().string();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(window, true);

#if defined(__APPLE__)
    ImGui_ImplOpenGL3_Init("#version 120");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif

    io.Fonts->AddFontDefault();
    static ImWchar ranges[] = {0x1, 0x1FFFF, 0};
    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.MergeMode = true;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    std::string fontPath = path.append("/assets/NotoEmoji.ttf");
    std::cout << "Font path: " << fontPath << std::endl;
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 13.0f, &cfg, ranges);
    io.Fonts->Build();
}

void gui::Init() noexcept
{
    WebsocketState current_ws_state = ws ? ws->getState() : WebsocketState::UNINITIALIZED;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize({WIDTH, HEIGHT}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos({0, 0}, ImGuiCond_FirstUseEver);

    ImGui::Begin("WSC", &quit, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);

    ImGui::Spacing();
    ImGui::Text("Host:");
    ImGui::InputTextWithHint("##Host", "wss://echo.websocket.org", &hostInput);
    ImGui::SameLine();

    if (current_ws_state == WebsocketState::CONNECTED)
    {
        if (ImGui::Button("Disconnect"))
        {
            ws->close();
        }
    }
    else
    {
        if (ImGui::Button("Connect"))
        {
            std::cout << "Connecting to " << hostInput << std::endl;
            try
            {
                ws = new WSC(hostInput);
            }
            catch (const std::exception &e)
            {
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

    ImGui::BeginDisabled(ws == nullptr || ws->getState() != WebsocketState::CONNECTED);
    ImGui::Text("Messages:");
    ImGui::BeginChild("Messages", ImVec2(0, HEIGHT - 250), true);
    if (ws && ws->getState() == WebsocketState::CONNECTED)
    {
        auto messages = ws->getMessages();
        for (auto &message : messages)
        {
            std::string inputId = "m_messages";
            inputId.append(std::to_string(message.timestamp.time_since_epoch().count()));
            std::string input = (message.type == Message::MessageType::SENT ? "⬆ [" : "⬇ [") + message.getFormattedTimestamp() + "] " + message.content;
            int lineCount = std::count(input.begin(), input.end(), '\n') + 1;
            char *buffer = (char *)malloc(input.size() + 1);
            strcpy(buffer, input.c_str());

            ImGui::PushID(inputId.c_str());
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, message.type == Message::MessageType::SENT ? IM_COL32(255, 229, 163, 255) : IM_COL32(255, 255, 255, 255));
            ImGui::InputTextMultiline("", buffer, input.size() + 1, ImVec2(-FLT_MIN, (ImGui::GetTextLineHeight() * lineCount) + 7), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopID();
            free(buffer);
        }
    }
    ImGui::EndChild();

    static bool focusOnInput = false;
    if (focusOnInput)
    {
        ImGui::SetKeyboardFocusHere();
        focusOnInput = false;
    }
    bool send = ImGui::InputTextMultiline("##Send", &messageInput,
                                          ImVec2(ImGui::GetWindowWidth() - 100, ImGui::GetTextLineHeight() * 5),
                                          ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if (ImGui::Button("Send") || send)
    {
        if (messageInput.length() > 0)
        {
            if (ws && ws->getState() == WebsocketState::CONNECTED)
            {
                ws->sendMsg(messageInput);
                messageInput.clear();
            }
        }
        focusOnInput = true;
    }
    ImGui::EndDisabled();

    ImGui::End();
}

void gui::Render() noexcept
{
    ImGui::Render();
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void gui::DestroyImGui() noexcept
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}