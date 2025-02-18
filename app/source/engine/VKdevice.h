#ifndef INCLUDE_VULKANDEVICE_H_
#define INCLUDE_VULKANDEVICE_H_

#include "../_common.h"

namespace vkengine {

    struct VKDevice_
    {
        VkPhysicalDevice VKphysicalDevice;                  // 물리 디바이스 -> GPU Physical Handle
        VkDevice VKdevice;                                  // 논리 디바이스 -> GPU Logical Handle
        VkPhysicalDeviceProperties properties;              // 물리 디바이스 속성
        VkPhysicalDeviceFeatures features;                  // 물리 디바이스 기능
        VkPhysicalDeviceMemoryProperties memoryProperties;	// 메모리 속성
        QueueFamilyIndices queueFamilyIndices;              // 큐 패밀리 인덱스
        std::set<std::string> supportedExtensions;          // 지원되는 Extensions 기능

        explicit VKDevice_(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice);
        VkResult createLogicalDevice(const bool enableValidationLayers, const std::vector<const char*> deviceExtensions);
        ~VKDevice_();
    };
}


#endif // INCLUDE_VULKANDEVICE_H_