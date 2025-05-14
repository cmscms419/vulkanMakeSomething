#ifndef INCLUDE_VK_INPUT_EVENT_H_
#define INCLUDE_VK_INPUT_EVENT_H_

#include "../../common/common.h"
#include <GLFW/glfw3.h>

namespace vkengine {
    namespace input {
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
        void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    }
}
#endif // INCLUDE_VKINPUTEVENT_H_
