#ifndef INCLUDE_VKKEY_H_
#define INCLUDE_VKKEY_H_

#include "../_common.h"

namespace vkengine {
    namespace input {
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    }
}
#endif // INCLUDE_VKKEY_H_
