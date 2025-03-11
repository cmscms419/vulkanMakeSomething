#ifndef INCLUDE_MACROS_H_
#define INCLUDE_MACROS_H_

#include <windows.h>
#include <Lmcons.h>
#include <algorithm>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

constexpr const char* DEBUG_USER_NAME_0 = "alstj";
constexpr int UNIQUE_VERTEXTYPE = 1;

namespace vkengine {
    extern const bool enableValidationLayers;
}

#if DEBUG_
#define PRINT_TO_CONSOLE(text, ...)              \
{                                           \
    const std::string localUserName = UserName; \
    const std::string targetUserName = static_cast<const std::string>(DEBUG_USER_NAME_0); /* ���⿡ �ڽ��� ����� �̸��� �Է��ϼ��� */ \
    if (vkengine::enableValidationLayers && localUserName == targetUserName) { \
        printf(text, __VA_ARGS__);          \
    }                                       \
}                                           \

#define CHECK_RESULT(f)                                                  \
{                                                                        \
    const std::string localUserName = UserName; \
    const std::string targetUserName = static_cast<const std::string>(DEBUG_USER_NAME_0); /* ���⿡ �ڽ��� ����� �̸��� �Է��ϼ��� */ \
    bool res = (f);                                                     \
    if (vkengine::enableValidationLayers && localUserName == targetUserName) {        \
        if (res != true) {                                                  \
            printf("Fatal : function is %d in %s at line %d\n", res, __FILE__,  \
                __LINE__);                                                      \
        }                                                                           \
    }                                                                       \
    assert(res == true);                                                \
}                                                                      \

#define VK_CHECK_RESULT(f)                                               \
{                                                                        \
    const std::string localUserName = UserName; \
    const std::string targetUserName = static_cast<const std::string>(DEBUG_USER_NAME_0); /* ���⿡ �ڽ��� ����� �̸��� �Է��ϼ��� */ \
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

#define PRINT_TO_CONSOLE(text, ...)
#define CHECK_RESULT(f) f
#define VK_CHECK_RESULT(f) f

#endif // DEBUG_

extern const std::string UserName;

#endif // !INCLUDE_MACROS_H_
