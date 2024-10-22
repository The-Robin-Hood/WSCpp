#include "gui.h"
#include <iostream>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
    
    
    ImGui::Text("Host:");
    ImGui::SameLine();
    ImGui::InputTextWithHint("##Host","example.com", &hostInput);

    ImGui::Text("Port:");
    ImGui::SameLine();
    ImGui::InputTextWithHint("##Port","By default 80", &portInput);


    if (ImGui::Button("Connect"))
    {
        // Print host and port to the console
        std::cout << "Host: " << hostInput << ", Port: " << portInput << std::endl;

        // Example of adding the connection info to the output text
        outputText += "Connected to " + hostInput + " on port " + portInput + "\n";
    }

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