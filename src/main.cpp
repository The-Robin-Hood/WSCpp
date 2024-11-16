#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include "WSCLogger.h"
#include "gui.h"

int main() {
    WSCLogger::init("WSC++", "WSCLog.txt", spdlog::level::debug);

    WSCLog(info, "Starting WSC++");
    GUI &gui = GUI::getInstance();
    if (!gui.init()) {
        return -1;
    }

    while (!gui.shouldClose()) {
        glfwPollEvents();
        gui.update();
        gui.render();
        glfwSwapBuffers(gui.getWindow());
    }

    gui.closeGUI();
    WSCLog(info, "Exiting WSC++");

    return 0;
}
