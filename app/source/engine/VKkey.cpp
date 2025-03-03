#include "VKkey.h"
#include "VKengine.h"
#include "Camera.h"

using vkengine::VulkanEngine;
using vkengine::object::Camera;

namespace vkengine
{
    namespace input {
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            VulkanEngine* app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            
            if (action == GLFW_PRESS || action == GLFW_REPEAT) 
            {
                app->setKeyPressed(key, true);
            }
            else if (action == GLFW_RELEASE) 
            {
                app->setKeyPressed(key, false);
            }
        }
    }
}
