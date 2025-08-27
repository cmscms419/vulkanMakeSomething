#ifndef INCLUDE_MACROS_H_
#define INCLUDE_MACROS_H_

#include <windows.h>
#include <Lmcons.h>
#include <algorithm>

#include "common.h"
#define UNIQUE_VERTEXTYPE 1

constexpr const cChar* DEBUG_USER_NAME_0 = "alstj";

namespace vkengine {
    extern const cBool enableValidationLayers;

    inline const cInt getHash(const cString& str) {
        cInt hash = 0;
        for (cChar ch : str) {
            hash = (hash * 31) + ch; // 간단한 해시 함수
        }
        return hash;
    }

}

#if DEBUG_
#define _PRINT_TO_CONSOLE_(text, ...)              \
{                                           \
    const cString localUserName = UserName; \
    const cString targetUserName = static_cast<const cString>(DEBUG_USER_NAME_0); /* 여기에 자신의 사용자 이름을 입력하세요 */ \
    if (vkengine::enableValidationLayers && localUserName == targetUserName) { \
        printf(text, __VA_ARGS__);          \
    }                                       \
}                                           \

#define _CHECK_RESULT_(f)                                                  \
{                                                                        \
    const cString localUserName = UserName; \
    const cString targetUserName = static_cast<const cString>(DEBUG_USER_NAME_0); /* 여기에 자신의 사용자 이름을 입력하세요 */ \
    cBool res = (f);                                                     \
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
    const cString localUserName = UserName; \
    const cString targetUserName = static_cast<const cString>(DEBUG_USER_NAME_0); /* 여기에 자신의 사용자 이름을 입력하세요 */ \
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

extern const cString UserName;

#endif // !INCLUDE_MACROS_H_
