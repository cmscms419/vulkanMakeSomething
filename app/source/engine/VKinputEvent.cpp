#include "VKinputEvent.h"
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
                app->setMousePressed(GLFW_MOUSE_BUTTON_LEFT, true);
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                app->setMousePressed(GLFW_MOUSE_BUTTON_LEFT, false);
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                app->setMousePressed(GLFW_MOUSE_BUTTON_RIGHT, true);
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
                app->setMousePressed(GLFW_MOUSE_BUTTON_RIGHT, false);
                app->setCameraMoveCheck(false);
            }
            else
            {
                // �ٸ� ���콺 ��ư�� ���� ó��
                // ���� �������� ����
                _PRINT_TO_CONSOLE_("Mouse button %d action %d not handled yet.\n", button, action);
            }
        }
        
        void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
        {
            glfwGetCursorPos(window, &xpos, &ypos);
            
            int windowWidth, windowHeight;
            glfwGetWindowSize(window, &windowWidth, &windowHeight);

            VulkanEngine* app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));

            if (app->getCameraMoveStyle())
            {
                app->getCamera()->RotateScreenStandard(static_cast<cFloat>(xpos), static_cast<cFloat>(ypos), windowWidth, windowHeight);
            }
            else
            {
                if (app->getMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {

                    if (!app->getCameraMoveCheck())
                    {
                        app->setMousePosition(static_cast<cFloat>(xpos), static_cast<cFloat>(ypos));
                        app->setCameraMoveCheck(true);
                        return;
                    }

                    cFloat xoffset = app->getLastMouseX() - static_cast<cFloat>(xpos);
                    cFloat yoffset = app->getLastMouseY() - static_cast<cFloat>(ypos);

                    cVec3 force = cVec3(xoffset, yoffset, 0.0f);
                    app->getCamera()->RotateDeltaRotation(force);

                    app->setMousePosition(static_cast<cFloat>(xpos), static_cast<cFloat>(ypos));

                }

            }

        }
        
        void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
        {
            VulkanEngine* app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));

            if (yoffset > 0) {
                cFloat fov = app->getCamera()->getFov() - 1.0f;
                app->getCamera()->setFov(fov);
            }
            else if (yoffset < 0) {
                cFloat fov = app->getCamera()->getFov() + 1.0f;
                app->getCamera()->setFov(fov);
            }
        }
    }
}
