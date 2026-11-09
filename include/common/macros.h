#ifndef INCLUDE_MACROS_H_
#define INCLUDE_MACROS_H_

#include <string>

#define UNIQUE_VERTEXTYPE 1

// TextureTable (바인들리스 텍스처 테이블) 상수
// shader/materialCommon.glsl 에 한 벌 더 두었다. 한쪽만 고치면 안 된다.
// 바인들리스 배열의 크기. 셰이더의 materialTextures[] 선언과 같아야 한다.
#define MAX_TEXTURE_COUNT 512

// 0번 슬롯은 더미 텍스처가 차지한다 (파일은 data.h 의 DUMMY_TEXTURE_PATH).
// 실제 텍스처는 1번부터 들어간다. 덕분에 테이블이 절대 비지 않고,
// "텍스처 없음" 을 -1 이 아니라 유효한 인덱스로 표현할 수 있다.
// 셰이더는 index > DUMMY_TEXTURE_INDEX 로 없음을 판별한다.
#define DUMMY_TEXTURE_INDEX 0

constexpr const char* DEBUG_USER_NAME_0 = "alstj";

namespace vkengine {
    inline const int getHash(const std::string& str) {
        int hash = 0;
        for (char ch : str) {
            hash = (hash * 31) + ch; // 간단한 해시 함수
        }
        return hash;
    }
}

#if DEBUG_
#define _EXIT_WITH_MESSAGE_(mes, ...)          \
{                                           \
    const std::string localUserName = UserName; \
    const std::string targetUserName = static_cast<const std::string>(DEBUG_USER_NAME_0); /* 여기에 자신의 사용자 이름을 입력하세요 */ \
    if (localUserName == targetUserName) { \
        printf(mes, __VA_ARGS__);          \
        exit(-1);                          \
    }                                       \
}                                           \

#define _PRINT_TO_CONSOLE_(text, ...)              \
{                                           \
    const std::string localUserName = UserName; \
    const std::string targetUserName = static_cast<const std::string>(DEBUG_USER_NAME_0); /* 여기에 자신의 사용자 이름을 입력하세요 */ \
    if (localUserName == targetUserName) { \
        printf(text, __VA_ARGS__);          \
    }                                       \
}                                           \

#define _CHECK_RESULT_(f)                                                  \
{                                                                        \
    const std::string localUserName = UserName; \
    const std::string targetUserName = static_cast<const std::string>(DEBUG_USER_NAME_0); /* 여기에 자신의 사용자 이름을 입력하세요 */ \
    bool res = (f);                                                     \
    if (localUserName == targetUserName) {        \
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
    if (localUserName == targetUserName) {        \
    if (res != VK_SUCCESS) {                                             \
      printf("Fatal : VkResult is %d in %s at line %d\n", res, __FILE__, \
             __LINE__);                                                  \
    }                                                                    \
    }                                                                    \
      assert(res == VK_SUCCESS);                                         \
  }                                                                      \


#else

#define _EXIT_WITH_MESSAGE_(mes, ...)
#define _PRINT_TO_CONSOLE_(text, ...)
#define _CHECK_RESULT_(f) f
#define _VK_CHECK_RESULT_(f) f

#endif // DEBUG_

extern const std::string UserName;

#endif // !INCLUDE_MACROS_H_
