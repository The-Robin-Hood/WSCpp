#pragma once
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>

#include <memory>
#include <string>

#include "WSCLogger.h"
#include "imgui.h"
#include "imgui_custom.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "ui/ui.h"
#include "ws.h"

class WSC;

class GUI {
   public:
    struct Config {
        int width = 700;
        int height = 500;
        std::string title = "WSCpp";
        std::string defaultHostHint = "wss://192.168.0.175:8000";
        bool vsync = true;
    };

    // Singleton
    static GUI &getInstance() {
        static GUI instance;
        return instance;
    }
    GUI(const GUI &) = delete;
    GUI &operator=(const GUI &) = delete;

    bool init();
    void render();
    void update();
    void closeGUI();

    const Config &getConfig() const { return m_windowConfig; }
    GLFWwindow *getWindow() const { return m_window.get(); }
    bool shouldClose() const { return glfwWindowShouldClose(getWindow()); }

   private:
    explicit GUI(const Config &config) : m_windowConfig(config) {}
    GUI() : m_windowConfig() {}
    ~GUI();

    // Custom deleter for GLFW window
    struct WindowDeleter {
        void operator()(GLFWwindow *window) {
            if (window) {
                glfwDestroyWindow(window);
                glfwTerminate();
            }
        }
    };

    // Member variables
    Config m_windowConfig;
    std::unique_ptr<GLFWwindow, WindowDeleter> m_window;
    std::unique_ptr<WSC> m_websocket;
    std::unique_ptr<MessageQueue> m_allMessages;
    std::string m_hostInput;
    std::string m_sendMessageInput;

    const std::string m_basePath = WSCUtils::getBasePath();
    const std::string m_imagesPath = m_basePath + "/assets/images";
    const std::string m_fontsPath = m_basePath + "/assets/fonts";
    const std::string m_logoPath = m_imagesPath + "/logo.png";

    std::shared_ptr<WSCpp::UI::Resources::Image> m_logoImage;
    std::map<std::string, std::shared_ptr<WSCpp::UI::Resources::Image>> m_icons;
    std::map<std::string, std::map<std::string, ImFont *>> m_fonts;

    // Internal methods
    bool initWindow();
    bool initImGui();

    void titleBar();

    // ImGui rendering methods
    void renderMainWindow();
    void renderStatusBar();
};
