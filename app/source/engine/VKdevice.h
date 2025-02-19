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
        VkPhysicalDeviceMemoryProperties memoryProperties;  // �޸� �Ӽ�
        QueueFamilyIndices queueFamilyIndices;              // ť �йи� �ε���
        std::set<std::string> supportedExtensions;          // �����Ǵ� Extensions ���

        VkQueue graphicsVKQueue;                            // �׷��Ƚ� ť -> �׷��Ƚ� ����� ó���ϴ� ť
        VkQueue presentVKQueue;                             // ������Ʈ ť -> ������ �ý��۰� Vulkan�� �����ϴ� �������̽�

        explicit VKDevice_(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice);
        VkResult createLogicalDevice();
        ~VKDevice_();
    };
}


#endif // INCLUDE_VULKANDEVICE_H_