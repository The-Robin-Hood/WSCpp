#define STB_IMAGE_IMPLEMENTATION

#include "WSCLogger.h"
#include "gui.h"

int main() {
    WSCLogger::init("WSCpp", "WSCLog.txt", spdlog::level::debug);

    WSCLog(info, "Starting WSCpp");
    GUI &gui = GUI::getInstance();
    if (!gui.init()) {
        return -1;
    }

    while (!gui.shouldClose()) {
        glfwPollEvents();
        gui.events();
        gui.update();
        gui.render();
        glfwSwapBuffers(gui.getWindow());
    }

    gui.closeGUI();
    WSCLog(info, "Exiting WSCpp");

    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    return main();
}
#endif
