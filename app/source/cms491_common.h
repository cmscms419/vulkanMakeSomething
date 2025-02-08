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

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;
constexpr int MAX_FRAMES = 4;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr int CREATESURFACE_VKWIN32SURFACECREATEINFOKHR = 1;

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
    const uint32_t getGraphicsQueueFamilyIndex() {
        uint32_t target = -1;

        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = graphicsFamily;
        }

        return target;
    }
    const uint32_t getPresentQueueFamilyIndex() {
        uint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = presentFamily;
        }
        return target;
    }

    const bool isComplete() {
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

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    // 바인딩 설명을 반환하는 함수
    // 이 구조체의 멤버 변수가 어떻게 바인딩되는지 설명합니다.
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    // 어트리뷰트 설명을 반환하는 함수
    // 위치와 색상을 나타내는 두 개의 어트리뷰트가 있습니다.
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

namespace vkutil
{
    namespace helper
    {
        // 파일을 읽어오는 함수
        std::vector<char> readFile(const std::string& filename);
        uint32_t findMemoryType(
            uint32_t typeFilter,
            VkMemoryPropertyFlags properties,
            VkPhysicalDevice VKphysicalDevice);
    }
}

const std::vector<Vertex> testVectex = {
    {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};


#endif // INCLUDE_CMS419_COMMON_H