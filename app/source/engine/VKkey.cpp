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
            
            // ��ȿ�� Ű���� Ȯ��
            if (key < GLFW_KEY_A || key > GLFW_KEY_Z) {
                return; // ��ȿ���� ���� Ű�� ����
            }

            if (action == GLFW_PRESS || action == GLFW_REPEAT) 
            {
                app->setKeyPressed(key, true);
            }
            else if (action == GLFW_RELEASE) 
            {
                app->setKeyPressed(key, false);
            }
            else
            {

            }
        }
        void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            VulkanEngine* app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
             //   app->setMousePressed(GLFW_MOUSE_BUTTON_LEFT, true);
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
               // app->setMousePressed(GLFW_MOUSE_BUTTON_LEFT, false);
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                //app->setMousePressed(GLFW_MOUSE_BUTTON_RIGHT, true);
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
               // app->setMousePressed(GLFW_MOUSE_BUTTON_RIGHT, false);
            }
        }
        void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
        {
            
            glfwGetCursorPos(window, &xpos, &ypos);
            
            int windowWidth, windowHeight;
            glfwGetWindowSize(window, &windowWidth, &windowHeight);

            VulkanEngine* app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
            app->getCamera()->MoveRotate(xpos, ypos, windowWidth, windowHeight);
        }
    }
}
