#ifndef INCLUDE_MACROS_H_
#define INCLUDE_MACROS_H_

#include <windows.h>
#include <Lmcons.h>
#include <algorithm>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>
#define UNIQUE_VERTEXTYPE 1

constexpr const char* DEBUG_USER_NAME_0 = "alstj";

namespace vkengine {
    extern const bool enableValidationLayers;
}

#if DEBUG_
#define _PRINT_TO_CONSOLE_(text, ...)              \
{                                           \
    const std::string localUserName = UserName; \
    const std::string targetUserName = static_cast<const std::string>(DEBUG_USER_NAME_0); /* 여기에 자신의 사용자 이름을 입력하세요 */ \
    if (vkengine::enableValidationLayers && localUserName == targetUserName) { \
        printf(text, __VA_ARGS__);          \
    }                                       \
}                                           \

#define _CHECK_RESULT_(f)                                                  \
{                                                                        \
    const std::string localUserName = UserName; \
    const std::string targetUserName = static_cast<const std::string>(DEBUG_USER_NAME_0); /* 여기에 자신의 사용자 이름을 입력하세요 */ \
    bool res = (f);                                                     \
    if (vkengine::enableValidationLayers && localUserName == targetUserName) {        \
        if (res != true) {                                                  \
            printf("Fatal : function is %d in %s at line %d\n", res, __FILE__,  \
                __LINE__);                                                      \
        }                                                                           \
    }                                                                       \
    assert(res == true);                                                \
}                                                                      \

#define _VK_CHECK_RESULT_(f)                                               \
{                                                                        \
    const std::string localUserName = UserName; \
    const std::string targetUserName = static_cast<const std::string>(DEBUG_USER_NAME_0); /* 여기에 자신의 사용자 이름을 입력하세요 */ \
    VkResult res = (f);                                                  \
    if (vkengine::enableValidationLayers && localUserName == targetUserName) {        \
    if (res != VK_SUCCESS) {                                             \
      printf("Fatal : VkResult is %d in %s at line %d\n", res, __FILE__, \
             __LINE__);                                                  \
    }                                                                    \
    }                                                                    \
      assert(res == VK_SUCCESS);                                         \
  }                                                                      \

#else

#define _PRINT_TO_CONSOLE_(text, ...)
#define _CHECK_RESULT_(f) f
#define _VK_CHECK_RESULT_(f) f

#endif // DEBUG_

extern const std::string UserName;

#endif // !INCLUDE_MACROS_H_
