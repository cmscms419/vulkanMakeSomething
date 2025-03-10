#ifndef INCLUDE_TEST_COMMON_H
#define INCLUDE_TEST_COMMON_H

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
const std::string TEST_TEXTURE_PATH = "../../../../../../source/image.jpg";

#ifdef DEBUG_
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

namespace vkutil
{
    namespace helper_
    {
        // ������ �о���� �Լ�
        std::vector<char> readFile(const std::string& filename);

        // ���� ����̽��� Ȯ�� ����� �����ϴ��� Ȯ���ϴ� �Լ�
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice VKphysicalDevice);

        // ���۸� �����ϴ� �Լ�
        // ���۸� �����ϰ� �޸𸮸� �Ҵ��մϴ�.
        void createBuffer(VkDevice device,
            VkPhysicalDevice physicalDevice,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);

        // ���۸� �����ϴ� �Լ�
        void copyBuffer(
            VkDevice& VKdevice,
            VkCommandPool& VKcommandPool,
            VkQueue& graphicsVKQueue,
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size);

        // �̹����� �����ϴ� �Լ�
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

        //  �����Ϸ��� ��ɹ��۸� �����ϴ� �Լ�
        VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool);

        // ��ɹ��۸� �����ϴ� �Լ�
        void endSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkCommandBuffer& commandBuffer);

        // �̹��� ���̾ƿ��� ��ȯ�ϴ� �Լ�
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

        // �̹����� �����ϴ� �Լ�
        void copyBufferToImage(
            VkDevice& device,
            VkCommandPool& commandPool,
            VkQueue& graphicsQueue,
            VkBuffer buffer,
            VkImage image,
            uint32_t width,
            uint32_t height);

        // �̹��� �並 �����ϴ� �Լ�
        VkImageView createImageView(
            VkDevice& device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels
        );

        // Format�� �����ϴ��� Ȯ���ϴ� �Լ�
        VkFormat findSupportedFormat(
            VkPhysicalDevice physicalDevice,
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features);

        // ���� ������ ã�� �Լ�
        VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

        // ���ٽ� ������Ʈ�� ������ �ִ��� Ȯ���ϴ� �Լ�
        bool hasStencilComponent(VkFormat format);

        // Mipmaps�� �����ϴ� �Լ�
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

        // �ִ� ��� ������ ���ø� ���� ��ȯ�ϴ� �Լ�
        VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

        // setupCommandBuffer ���߿� �߰�
        // flushSetupCommands ���߿� �߰�

    }
}

#endif // INCLUDE_TEST_COMMON_H