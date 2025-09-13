#ifndef INCLUDE_VULKAN_DEVICE_2_H_
#define INCLUDE_VULKAN_DEVICE_2_H_

#include "common.h"
#include "struct.h"
#include "helper.h"

#include "log.h"

namespace vkengine {

    struct VKdeviceHandler2
    {
    public:
        VKdeviceHandler2() = default;
        VKdeviceHandler2(VkPhysicalDevice physicalDevice, QueueFamilyIndices2 indice);
        ~VKdeviceHandler2();

        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };                  // ���� ����̽� -> GPU Physical Handle
        VkDevice logicaldevice{ VK_NULL_HANDLE };                                  // �� ����̽� -> GPU Logical Handle
        VkPhysicalDeviceProperties properties{};                              // ���� ����̽� �Ӽ�
        VkPhysicalDeviceFeatures features{};                                  // ���� ����̽� ���
        VkPhysicalDeviceFeatures enabledFeatures{};                           // Ȱ��ȭ�� ���� ����̽� ���
        VkPhysicalDeviceMemoryProperties memoryProperties{};                  // �޸� �Ӽ�
        QueueFamilyIndices2 queueFamilyIndices{};                              // ť �йи� �ε���
        std::set<std::string> supportedExtensions;                            // �����Ǵ� Extensions ���
        std::vector<const char*> enabledDeviceExtensions{};                     // Ȱ��ȭ�� ����̽� Ȯ�� ���

        VkCommandPool graphicsCommandPool{ VK_NULL_HANDLE };                        // Ŀ�ǵ� Ǯ -> Ŀ�ǵ� ���۸� �����ϴ� �� ���
        VkCommandPool computeCommandPool{ VK_NULL_HANDLE };                        // Ŀ�ǵ� Ǯ -> Ŀ�ǵ� ���۸� �����ϴ� �� ���
        VkCommandPool transferCommandPool{ VK_NULL_HANDLE };                        // Ŀ�ǵ� Ǯ -> Ŀ�ǵ� ���۸� �����ϴ� �� ���

        VkQueue graphicsVKQueue{ VK_NULL_HANDLE };                            // �׷��Ƚ� ť -> �׷��Ƚ� ����� ó���ϴ� ť
        VkQueue computerVKQueue{ VK_NULL_HANDLE };                            // ��ǻƮ ť -> ��ǻƮ ����� ó���ϴ� ť
        VkQueue transferVKQueue{ VK_NULL_HANDLE };                             // ������Ʈ ť -> ������ �ý��۰� Vulkan�� �����ϴ� �������̽�

        //VkResult createPysicalDevice
        void printPysicaldeviceProperties() const;

        cBool createLogicalDevice(cBool useSwapChain);
        cBool createCommandPools();
        cBool createQueues();

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
            VkDeviceMemory& imageMemory);
        const VkShaderModule createShaderModule(const std::string& path) const;

        void cleanup();
    };
}


#endif // INCLUDE_VULKAN_DEVICE_2_H_