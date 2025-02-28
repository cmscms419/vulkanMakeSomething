#include "VKkey.h"
#include "VKengine.h"

using vkengine::VulkanEngine;

namespace vkengine
{
    namespace input {
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            VulkanEngine* app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                PRINT_TO_CONSOLE("W key pressed or repeat");
            }
            if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                PRINT_TO_CONSOLE("A key pressed or repeat");
            }
            if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                PRINT_TO_CONSOLE("S key pressed or repeat");
            }
            if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
                PRINT_TO_CONSOLE("D key pressed or repeat");
            }
        }
    }
}
