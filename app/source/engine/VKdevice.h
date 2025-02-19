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
        VkPhysicalDeviceMemoryProperties memoryProperties;  // 메모리 속성
        QueueFamilyIndices queueFamilyIndices;              // 큐 패밀리 인덱스
        std::set<std::string> supportedExtensions;          // 지원되는 Extensions 기능

        VkQueue graphicsVKQueue;                            // 그래픽스 큐 -> 그래픽스 명령을 처리하는 큐
        VkQueue presentVKQueue;                             // 프레젠트 큐 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스

        explicit VKDevice_(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice);
        VkResult createLogicalDevice();
        ~VKDevice_();
    };
}


#endif // INCLUDE_VULKANDEVICE_H_