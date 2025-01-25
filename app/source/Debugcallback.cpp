
#include "../../include/source/Debugcallback.h"

using namespace vkutil;

namespace vkutil {

    // vkutil namespace에 key_callback 함수 정의
    void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
        Application *app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
} // namespace vkutil