#ifndef INCLUDE_VULKAN_DEVICE_H_
#define INCLUDE_VULKAN_DEVICE_H_

#include "../../common/common.h"
#include "../../common/struct.h"

#include <set>

namespace vkengine {

    struct VKDevice_
    {
    public:
        explicit VKDevice_(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice);
        ~VKDevice_() = default;

        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };                  // ���� ����̽� -> GPU Physical Handle
        VkDevice logicaldevice{ VK_NULL_HANDLE };                                  // �� ����̽� -> GPU Logical Handle
        VkPhysicalDeviceProperties properties{};                              // ���� ����̽� �Ӽ�
        VkPhysicalDeviceFeatures features{};                                  // ���� ����̽� ���
        VkPhysicalDeviceMemoryProperties memoryProperties{};                  // �޸� �Ӽ�
        QueueFamilyIndices queueFamilyIndices{};                              // ť �йи� �ε���
        std::set<std::string> supportedExtensions;                            // �����Ǵ� Extensions ���
        VkCommandPool commandPool{ VK_NULL_HANDLE };                        // Ŀ�ǵ� Ǯ -> Ŀ�ǵ� ���۸� �����ϴ� �� ���
        VkQueue graphicsVKQueue{ VK_NULL_HANDLE };                            // �׷��Ƚ� ť -> �׷��Ƚ� ����� ó���ϴ� ť
        VkQueue presentVKQueue{ VK_NULL_HANDLE };                             // ������Ʈ ť -> ������ �ý��۰� Vulkan�� �����ϴ� �������̽�

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

        void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
        void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin);
        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin);

        void cleanup() const;
    };
}


#endif // INCLUDE_VULKANDEVICE_H_