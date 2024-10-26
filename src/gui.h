#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>
#include <string>
#include <ws.h>

class WSC;

namespace gui {

    constexpr int WIDTH = 700;
    constexpr int HEIGHT = 500;

    inline bool quit = true;

    inline GLFWwindow *window = nullptr;

    void CreateGlfWindow(const char *title) noexcept;
    void DestroyGlfWindow() noexcept;

    void CreateImGui() noexcept;
    void Init() noexcept;
    void Render() noexcept;
    void DestroyImGui() noexcept;


    inline std::string hostInput = "wss://localhost:4000";
    inline std::string messageInput = "";
    extern WSC* ws;
};
