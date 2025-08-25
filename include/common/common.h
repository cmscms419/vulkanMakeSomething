#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

//#include <algorithm>
//#include <array>
//#include <chrono>
//#include <cstdlib>
//#include <cstring>
//#include <fstream>
//#include <set>
//#include <stdexcept>
//#include <vector>
//#include <map>

#include <chrono>
#include <iostream>

#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#define HASH_ID(str) vkengine::hash(str)

typedef bool cBool;
typedef char cChar;
typedef unsigned char cUChar;
typedef int cInt;
typedef unsigned int cUint;
typedef long cLong;
typedef unsigned long cULong;
typedef long long cLongLong;
typedef unsigned long long cULongLong;
typedef size_t cSize;
typedef uint8_t cUInt8_t;
typedef uint16_t cUint16_t;
typedef uint32_t cUint32_t;
typedef uint64_t cUint64_t;
typedef int8_t cInt8_t;
typedef int16_t cInt16_t;
typedef int32_t cInt32_t;
typedef int64_t cInt64_t;
typedef float cFloat;
typedef double cDouble;
typedef std::string cString;
typedef glm::vec2 cVec2;
typedef glm::vec3 cVec3;
typedef glm::vec4 cVec4;
typedef glm::mat4 cMat4;
typedef glm::quat cQuat;

constexpr cInt WIDTH = 1280;
constexpr cInt HEIGHT = 720;
constexpr cInt MAX_FRAMES = 4;
constexpr cInt MAX_FRAMES_IN_FLIGHT = 2;
constexpr cInt MAX_FRAMES_IN_FLIGHT_UI_VERSION = 3;
constexpr cInt CREATESURFACE_VKWIN32SURFACECREATEINFOKHR = 0;

constexpr cFloat XM_PI = 3.141592654f;
constexpr cFloat XM_2PI = 6.283185307f;

namespace vkengine
{


    extern const std::vector<const char*> validationLayers;
    extern const std::vector<const char*> deviceExtensions;
    extern const std::vector<VkDynamicState> dynamicStates;
    extern const cString RESOURSE_PATH;
    extern const cString MODEL_PATH;
    extern const cString TEXTURE_PATH;
    extern const cString TEST_TEXTURE_PATH;
    extern const cString TEST_TEXTURE_PATH_ARRAY0;
    extern const cString TEST_TEXTURE_PATH_ARRAY1;
    extern const cString TEST_TEXTURE_PATH_ARRAY2;
    extern const cString TEST_TEXTURE_PATH_ARRAY3;
    extern const cString TEST_TEXTURE_PATH_ARRAY4;
    extern const cString TEST_TEXTURE_PATH_ARRAY5;
    extern const cString CUBE_TEXTURE_PATH;
    extern const cBool enableValidationLayers;

} // namespace vkengine

#endif // INCLUDE_COMMON_H