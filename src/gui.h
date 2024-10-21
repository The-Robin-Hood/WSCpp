#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace gui {

    constexpr int WIDTH = 800;
    constexpr int HEIGHT = 600;

    inline bool quit = true;

    inline GLFWwindow *window = nullptr;

    void CreateWindow(const char *title) noexcept;
    void DestroyWindow() noexcept;

    void CreateImGui() noexcept;
    void Init() noexcept;
    void Render() noexcept;
    void DestroyImGui() noexcept;


    inline std::string hostInput = "";
    inline std::string portInput = "";
    inline std::string outputText = ""; // Buffer for output text
}
