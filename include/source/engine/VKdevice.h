#ifndef INCLUDE_VULKAN_DEVICE_H_
#define INCLUDE_VULKAN_DEVICE_H_

#include "common.h"
#include "struct.h"

#include <set>

namespace vkengine {

    struct VKdeviceHandler
    {
    public:
        explicit VKdeviceHandler(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice);
        ~VKdeviceHandler() = default;

        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };                  // 물리 디바이스 -> GPU Physical Handle
        VkDevice logicaldevice{ VK_NULL_HANDLE };                                  // 논리 디바이스 -> GPU Logical Handle
        VkPhysicalDeviceProperties properties{};                              // 물리 디바이스 속성
        VkPhysicalDeviceFeatures features{};                                  // 물리 디바이스 기능
        VkPhysicalDeviceMemoryProperties memoryProperties{};                  // 메모리 속성
        QueueFamilyIndices queueFamilyIndices{};                              // 큐 패밀리 인덱스
        std::set<std::string> supportedExtensions;                            // 지원되는 Extensions 기능
        VkCommandPool commandPool{ VK_NULL_HANDLE };                        // 커맨드 풀 -> 커맨드 버퍼를 생성하는 데 사용
        VkQueue graphicsVKQueue{ VK_NULL_HANDLE };                            // 그래픽스 큐 -> 그래픽스 명령을 처리하는 큐
        VkQueue presentVKQueue{ VK_NULL_HANDLE };                             // 프레젠트 큐 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스

        VkResult createLogicalDevice();
        void createimageview(
            uint32_t width,
            uint32_t height,
            uint32_t mipLevels,
            VkSampleCountFlagBits numSamples,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory) const;
        const VkShaderModule createShaderModule(const std::string& path) const;

        void cleanup() const;
    };
}


#endif // INCLUDE_VULKANDEVICE_H_