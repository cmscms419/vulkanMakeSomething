#ifndef INCLUDE_VULKANDEVICE_H_
#define INCLUDE_VULKANDEVICE_H_

#include "../_common.h"

namespace vkengine {

    struct VKDevice_
    {
    public:
        ~VKDevice_();
        VkPhysicalDevice VKphysicalDevice;                  // ���� ����̽� -> GPU Physical Handle
        VkDevice VKdevice;                                  // �� ����̽� -> GPU Logical Handle
        VkPhysicalDeviceProperties properties;              // ���� ����̽� �Ӽ�
        VkPhysicalDeviceFeatures features;                  // ���� ����̽� ���
        VkPhysicalDeviceMemoryProperties memoryProperties;  // �޸� �Ӽ�
        QueueFamilyIndices queueFamilyIndices;              // ť �йи� �ε���
        std::set<std::string> supportedExtensions;          // �����Ǵ� Extensions ���
        VkCommandPool VKcommandPool{ VK_NULL_HANDLE };      // Ŀ�ǵ� Ǯ -> Ŀ�ǵ� ���۸� �����ϴ� �� ���

        VkQueue graphicsVKQueue;                            // �׷��Ƚ� ť -> �׷��Ƚ� ����� ó���ϴ� ť
        VkQueue presentVKQueue;                             // ������Ʈ ť -> ������ �ý��۰� Vulkan�� �����ϴ� �������̽�

        explicit VKDevice_(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice);
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
            VkDeviceMemory& imageMemory);
    };
}


#endif // INCLUDE_VULKANDEVICE_H_