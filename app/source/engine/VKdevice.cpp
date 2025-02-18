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

        vkGetPhysicalDeviceProperties(this->VKphysicalDevice, &properties);       // ���� ����̽� �Ӽ� ��������
        vkGetPhysicalDeviceFeatures(this->VKphysicalDevice, &features);           // ���� ����̽� ��� ��������
        vkGetPhysicalDeviceMemoryProperties(VKphysicalDevice, &memoryProperties); // �޸� �Ӽ� ��������

        uint32_t supportedApiVersion = properties.apiVersion;

        if (supportedApiVersion >= VK_API_VERSION_1_2) {
            // Vulkan 1.2 �̻��� �����ϴ� ����̽� ó��
            // ��: ���ο� ��� Ȱ��ȭ
        }
        else if (supportedApiVersion >= VK_API_VERSION_1_1) {
            // Vulkan 1.1�� �����ϴ� ����̽� ó��
        }
        else {
            // Vulkan 1.0�� �����ϴ� ����̽� ó��
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