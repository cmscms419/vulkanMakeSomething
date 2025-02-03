#ifndef INCLUDE_CMS419_COMMON_H
#define INCLUDE_CMS419_COMMON_H

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

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

//#ifdef _WIN32
//#include <windows.h>
//#elif __linux__
//#include <unistd.h>
//#include <limits.h>
//#endif

#include <glm/glm.hpp>

#define WIDTH  800
#define HEIGHT  600

const int MAX_FRAMES_IN_FLIGHT = 2;


//#ifdef _WIN32
//
//#elif __linux__
//
//#else
//#error "Unsupported platform"
//
//#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
};

const std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

#ifdef DEBUG_
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

struct QueueFamilyIndices {
    uint32_t graphicsFamily = 0;        // 그래픽스 큐 패밀리 인덱스 (그래픽스 명령을 처리하는 큐)
    uint32_t presentFamily = 0;         // 프레젠트 큐 패밀리 인덱스 (윈도우 시스템과 Vulkan을 연결하는 인터페이스)
    VkQueueFamilyProperties queueFamilyProperties = {};

    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;

    void setGraphicsFamily(uint32_t index) {
        graphicsFamily = index;
        graphicsFamilyHasValue = true;
    }
    void setPresentFamily(uint32_t index) {
        presentFamily = index;
        presentFamilyHasValue = true;
    }
    uint32_t getGraphicsQueueFamilyIndex() {
        uint32_t target = -1;

        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = graphicsFamily;
        }

        return target;
    }
    uint32_t getPresentQueueFamilyIndex() {
        uint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = presentFamily;
        }
        return target;
    }

    bool isComplete() {
        return this->graphicsFamilyHasValue && this->presentFamilyHasValue;
    }
    void reset() {
        this->graphicsFamily = 0;
        this->presentFamily = 0;
        this->graphicsFamilyHasValue = false;
        this->presentFamilyHasValue = false;
        this->queueFamilyProperties = {};
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// 파일을 읽어오는 함수
static std::vector<char> readFile(const std::string& filename) {

    // 파일 끝으로 이동하여 파일 크기를 가져옵니다.
    std::ifstream file(filename, std::ios::ate | std::ios::binary); 

    // 파일을 열 수 없는 경우 예외를 발생시킵니다.
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    
    size_t fileSize = (size_t)file.tellg(); // 파일 크기를 이용하여 버퍼를 할당합니다.
    std::vector<char> buffer(fileSize);     // 파일 포인터를 파일의 시작으로 이동합니다.
    file.seekg(0);                          // 파일 포인터를 파일의 시작으로 이동합니다.
    file.read(buffer.data(), fileSize);     // 파일 내용을 버퍼에 읽어옵니다. -> 파일을 fileSize 크기만큼 한번에 읽어온다.
    file.close();                           // 파일을 닫습니다.

    return buffer;
}

#endif // INCLUDE_CMS419_COMMON_H