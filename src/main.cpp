#include "gui.h"

int main(int argc, char **argv) {
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
