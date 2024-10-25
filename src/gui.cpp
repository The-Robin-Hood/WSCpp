#include "gui.h"
#include <iostream>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_custom.h"

#include "ws.h"

void gui::CreateWindow(const char *title) noexcept
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

void gui::DestroyWindow() noexcept
{
    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
}

void gui::CreateImGui() noexcept
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    # if defined(__APPLE__)
    ImGui_ImplOpenGL3_Init("#version 120");
    # else
    ImGui_ImplOpenGL3_Init("#version 130");
    # endif

}

void gui::Init() noexcept
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize({WIDTH, HEIGHT}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos({0, 0}, ImGuiCond_FirstUseEver);

    ImGui::Begin("WSC", &quit, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    
    ImGui::Spacing();
    ImGui::Text("Host:");
    ImGui::InputTextWithHint("##Host","example.com", &hostInput);
    ImGui::SameLine();

    if (ImGui::Button("Connect"))
    {

        ws::Init();
        if (ws::Connect(hostInput))
        {
            outputText += "Connected to " + hostInput + "\n";
        }
        else
        {
            outputText += "Failed to connect to " + hostInput + "\n";
        }
        
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::Text("Status : ");
    ImGui::SameLine();
    if(ws::currentState == ws::ws_state::UNINITIALIZED)
        ImGui::TextColored(RGBAtoIV4(200, 200, 200, 0.5), "Uninitialized");
    else if(ws::currentState == ws::ws_state::CONNECTING)
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connecting");
    else if(ws::currentState == ws::ws_state::DISCONNECTED)
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
    else if(ws::currentState == ws::ws_state::CONNECTED)
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
    
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    // Text container for output messages
    ImGui::Text("Messages:");
    ImGui::BeginChild("Messages", ImVec2(0, HEIGHT - 150), true); // Resizable child window
    ImGui::TextWrapped("%s", outputText.c_str());                 // Display output text
    ImGui::EndChild();

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