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
//#include <unordered_map>

#include <chrono>
#include <iostream>

#include <vector>
#include <string>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr int MAX_FRAMES = 4;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr int MAX_FRAMES_IN_FLIGHT_UI_VERSION = 3;
constexpr int CREATESURFACE_VKWIN32SURFACECREATEINFOKHR = 0;

namespace vkengine
{
    extern const std::vector<const char*> validationLayers;
    extern const std::vector<const char*> deviceExtensions;
    extern const std::vector<VkDynamicState> dynamicStates;
    extern const std::string RESOURSE_PATH;
    extern const std::string MODEL_PATH;
    extern const std::string TEXTURE_PATH;
    extern const std::string TEST_TEXTURE_PATH;
    extern const std::string TEST_TEXTURE_PATH_ARRAY0;
    extern const std::string TEST_TEXTURE_PATH_ARRAY1;
    extern const std::string TEST_TEXTURE_PATH_ARRAY2;
    extern const std::string TEST_TEXTURE_PATH_ARRAY3;
    extern const std::string TEST_TEXTURE_PATH_ARRAY4;
    extern const std::string TEST_TEXTURE_PATH_ARRAY5;
    extern const std::string CUBE_TEXTURE_PATH;
    extern const bool enableValidationLayers;

} // namespace vkengine

#endif // INCLUDE_COMMON_H