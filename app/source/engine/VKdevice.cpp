#include "VKdevice.h"
#include "helper.h"

namespace vkengine {

    using namespace helper;

    VKDevice_::VKDevice_(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice)
    {
        assert(physicalDevice);
        assert(indice.isComplete());

        this->VKphysicalDevice = physicalDevice;
        this->queueFamilyIndices = indice;

        vkGetPhysicalDeviceProperties(this->VKphysicalDevice, &properties);       // 물리 디바이스 속성 가져오기
        vkGetPhysicalDeviceFeatures(this->VKphysicalDevice, &features);           // 물리 디바이스 기능 가져오기
        vkGetPhysicalDeviceMemoryProperties(VKphysicalDevice, &memoryProperties); // 메모리 속성 가져오기

        uint32_t supportedApiVersion = properties.apiVersion;

        if (supportedApiVersion >= VK_API_VERSION_1_2) {
            // Vulkan 1.2 이상을 지원하는 디바이스 처리
            // 예: 새로운 기능 활성화
        }
        else if (supportedApiVersion >= VK_API_VERSION_1_1) {
            // Vulkan 1.1을 지원하는 디바이스 처리
        }
        else {
            // Vulkan 1.0을 지원하는 디바이스 처리
        }

#ifdef DEBUG_
        printf("Select Device\n");
        printf("Select DeviceProperties.deviceType: %d\n", properties.deviceType);
        printf("Select Device Name: %s\n", properties.deviceName);
#endif // DEBUG_

        helper::getDeviceExtensionSupport(physicalDevice, &supportedExtensions);
    }

    VKDevice_::~VKDevice_()
    {
    }
}