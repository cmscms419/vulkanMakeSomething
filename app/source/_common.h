#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

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
#include <unordered_map>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "../source/engine/imgui.h"
#include "../source/engine/imgui_impl_glfw.h"
#include "../source/engine/imgui_impl_vulkan.h"

//#ifdef _WIN32
//#include <windows.h>
//#elif __linux__
//#include <unistd.h>
//#include <limits.h>
//#endif

constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr int MAX_FRAMES = 4;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr int CREATESURFACE_VKWIN32SURFACECREATEINFOKHR = 0;

//#ifdef _WIN32
//
//#elif __linux__
//
//#else
//#error "Unsupported platform"
//
//#endif

#define UNIQUE_VERTEXTYPE 1

#define CHECK_RESULT(f)                                                 \
{                                                                        \
    bool res = (f);                                                      \
    if (res != true) {                                                   \
      printf("Fatal : function is %d in %s at line %d\n", res, __FILE__, \
             __LINE__);                                                  \
      assert(res == true);                                               \
    }                                                                    \
  }                                                                      \

#define VK_CHECK_RESULT(f)                                               \
{                                                                        \
    VkResult res = (f);                                                  \
    if (res != VK_SUCCESS) {                                             \
      printf("Fatal : VkResult is %d in %s at line %d\n", res, __FILE__, \
             __LINE__);                                                  \
      assert(res == VK_SUCCESS);                                         \
    }                                                                    \
  }                                                                      \

#define PRINT_TO_CONSOLE(text)              \
{                                           \
    printf("%s \n", &text);                 \
}                                           \


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

const std::string MODEL_PATH = "../../../../../../source/viking_room.obj";
const std::string TEXTURE_PATH = "../../../../../../source/viking_room.png";

#ifdef DEBUG_
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

namespace vkutil
{
    namespace helper_
    {
        // 파일을 읽어오는 함수
        std::vector<char> readFile(const std::string& filename);
        
        // 물리 디바이스의 확장 기능을 지원하는지 확인하는 함수
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice VKphysicalDevice);
        
        // 버퍼를 생성하는 함수
        // 버퍼를 생성하고 메모리를 할당합니다.
        void createBuffer(VkDevice device, 
            VkPhysicalDevice physicalDevice, 
            VkDeviceSize size, 
            VkBufferUsageFlags usage, 
            VkMemoryPropertyFlags properties, 
            VkBuffer& buffer, 
            VkDeviceMemory& bufferMemory);

        // 버퍼를 복사하는 함수
        void copyBuffer(
            VkDevice& VKdevice,
            VkCommandPool& VKcommandPool,
            VkQueue& graphicsVKQueue,
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size);
        
        // 이미지를 생성하는 함수
        void createImage(
            VkDevice& VKdevice,
            VkPhysicalDevice& VKphysicalDevice,
            uint32_t width,
            uint32_t height,
            uint32_t mipLevels,
            VkSampleCountFlagBits numSamples,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory);

        //  시작하려는 명령버퍼를 생성하는 함수
        VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool);
    
        // 명령버퍼를 종료하는 함수
        void endSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkCommandBuffer& commandBuffer);

        // 이미지 레이아웃을 전환하는 함수
        void transitionImageLayout(
            VkDevice& device,
            VkCommandPool& commandPool,
            VkQueue& graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels
        );

        // 이미지를 복사하는 함수
        void copyBufferToImage(
            VkDevice& device,
            VkCommandPool& commandPool,
            VkQueue& graphicsQueue,
            VkBuffer buffer,
            VkImage image,
            uint32_t width,
            uint32_t height);

        // 이미지 뷰를 생성하는 함수
        VkImageView createImageView(
            VkDevice& device,
            VkImage image,
            VkFormat format, 
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels
        );

        // Format을 지원하는지 확인하는 함수
        VkFormat findSupportedFormat(
            VkPhysicalDevice physicalDevice,
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features);

        // 깊이 형식을 찾는 함수
        VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

        // 스텐실 컴포넌트를 가지고 있는지 확인하는 함수
        bool hasStencilComponent(VkFormat format);

        // Mipmaps을 생성하는 함수
        void generateMipmaps(
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat imageFormat,
            int32_t texWidth,
            int32_t texHeight,
            uint32_t mipLevels);

        // 최대 사용 가능한 샘플링 수를 반환하는 함수
        VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

        // setupCommandBuffer 나중에 추가
        // flushSetupCommands 나중에 추가
            
    }
}



#endif // INCLUDE_COMMON_H