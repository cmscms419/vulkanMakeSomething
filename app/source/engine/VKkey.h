#ifndef INCLUDE_VKKEY_H_
#define INCLUDE_VKKEY_H_

#include "../_common.h"

namespace vkengine {
    namespace input {
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    }
}
#endif // INCLUDE_VKKEY_H_
