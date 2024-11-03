#define _CRT_SECURE_NO_WARNINGS
#include "gui.h"

int main() {
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
    return 0;
}
