#include <iostream>
#include "gui.h"

int main(int argc, char **argv)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    gui::CreateGlfWindow("WSC++");
    gui::CreateImGui();

    while (!glfwWindowShouldClose(gui::window))
    {
        gui::Init();
        gui::Render();
        glfwSwapBuffers(gui::window);
        glfwPollEvents();
    }

    gui::DestroyImGui();
    gui::DestroyGlfWindow();

    return 0;
}
