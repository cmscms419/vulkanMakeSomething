#ifndef INCLUDE_VULKANDEVICE_H_
#define INCLUDE_VULKANDEVICE_H_

#include "../_common.h"

namespace vkengine {

    struct VKDevice_
    {
        VkPhysicalDevice VKphysicalDevice;                  // ���� ����̽� -> GPU Physical Handle
        VkDevice VKdevice;                                  // �� ����̽� -> GPU Logical Handle
        VkPhysicalDeviceProperties properties;              // ���� ����̽� �Ӽ�
        VkPhysicalDeviceFeatures features;                  // ���� ����̽� ���
        VkPhysicalDeviceMemoryProperties memoryProperties;	// �޸� �Ӽ�
        QueueFamilyIndices queueFamilyIndices;              // ť �йи� �ε���
        std::set<std::string> supportedExtensions;          // �����Ǵ� Extensions ���

        explicit VKDevice_(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice);
        VkResult createLogicalDevice(const bool enableValidationLayers, const std::vector<const char*> deviceExtensions);
        ~VKDevice_();
    };
}


#endif // INCLUDE_VULKANDEVICE_H_