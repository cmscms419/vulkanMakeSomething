#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
#include <map>

#define WIDTH  800
#define HEIGHT  600

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
//#ifdef _WIN64
//#include <vulkan/vulkan_win32.h>
//#endif

#include <glm/glm.hpp>

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef DEBUG_
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

struct QueueFamilyIndices {
    std::vector<uint32_t> graphicsFamily = {};

    bool isEmpty() {
        return !graphicsFamily.empty();
    }
};