#include "VKkey.h"
#include "VKengine.h"

using vkcms::VulkanEngine;

namespace vkcms
{
    namespace input {
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            VulkanEngine* app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
    }
}
