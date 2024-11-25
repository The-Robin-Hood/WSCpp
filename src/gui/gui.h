#pragma once
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>

#include <memory>
#include <string>

#include "WSCLogger.h"
#include "ui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_custom.h"
#include "imgui_freetype.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "ws.h"

#define WCHAR(x) (ImWchar)(x)

class WSC;

class GUI {
   public:
    struct Config {
        int width = 700;
        int height = 500;
        std::string title = "WSC++";
        std::string defaultHostHint = "wss://192.168.0.175:8000";
        bool vsync = true;
        bool resizable = true;
    };

    // Singleton
    static GUI &getInstance() {
        static GUI instance;
        return instance;
    }

    // Builder pattern for configuration
    // class Builder {
    //    public:
    //     Builder() : m_config{} {}
    //     Builder &setSize(int width, int height) {
    //         m_config.width = width;
    //         m_config.height = height;
    //         return *this;
    //     }
    //     Builder &setTitle(const std::string &title) {
    //         m_config.title = title;
    //         return *this;
    //     }
    //     Builder &setHost(const std::string &host) {
    //         m_config.defaultHostHint = host;
    //         return *this;
    //     }
    //     Builder &setVSync(bool enabled) {
    //         m_config.vsync = enabled;
    //         return *this;
    //     }
    //     Builder &setResizable(bool resizable) {
    //         m_config.resizable = resizable;
    //         return *this;
    //     }
    //     GUI build() { return GUI(m_config); }

    //    private:
    //     Config m_config;
    // };

    // Main interface
    bool init();
    void render();
    void update();
    bool shouldClose() const;
    void closeGUI();

    // Getters
    const Config &getConfig() const { return m_config; }
    GLFWwindow *getWindow() const { return m_window.get(); }

    GUI(const GUI &) = delete;
    GUI &operator=(const GUI &) = delete;

   private:
    explicit GUI(const Config &config) : m_config(config) {}
    GUI() : m_config() {}
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
    Config m_config;
    std::unique_ptr<GLFWwindow, WindowDeleter> m_window;
    std::unique_ptr<WSC> m_websocket;
    std::unique_ptr<MessageQueue> m_allMessages;
    std::string m_hostInput;
    std::string m_sendMessageInput;

    std::string m_basePath = WSCUtils::getBasePath();
    std::string m_imagesPath = m_basePath + "/assets/images";
    std::string m_fontsPath = m_basePath + "/assets/fonts";

    std::vector<int> m_fontSizes = {12, 14, 16, 20, 24};
    std::vector<std::string> m_fontNames = {"NotoEmoji", "Inter-Regular",
                                            "Inter-SemiBold", "Inter-Bold"};
    std::map<std::string, std::map<std::string, ImFont *>> m_fonts;

#ifdef _WIN32
    std::vector<char> m_preLoadedlogo = WSCUtils::loadResource(101, "PNG");
    std::vector<std::vector<char>> m_preLoadedfonts = {
        WSCUtils::loadResource(102, "BINARY"), WSCUtils::loadResource(103, "BINARY"),
        WSCUtils::loadResource(104, "BINARY"), WSCUtils::loadResource(105, "BINARY")
        };
#else
    std::string m_logoPath = m_imagesPath + "/logo.png";
#endif

    GLuint m_logoTexture;

    // Internal methods
    bool initWindow();
    bool setWindowIcon();
    bool initImGui();
    void cleanupImGui();

    // Layout methods
    void beginBaseLayout();
    void endBaseLayout();

    void titleBar();

    // ImGui rendering methods
    void renderMainWindow();
    void renderStatusBar();
};
