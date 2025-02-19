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

    VkResult VKDevice_::createLogicalDevice()
    {
        // 1. ���� ��ġ���� ť �йи� �ε����� ã���ϴ�.
        // 2. ť ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        // 3. ���� ��ġ ��� ����ü�� �ʱ�ȭ�մϴ�.
        // 4. �� ��ġ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        // 5. �� ��ġ�� �����մϴ�.
        VkResult result = VK_SUCCESS;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::set<uint32_t> uniqueQueueFamilies = {
            this->queueFamilyIndices.graphicsAndComputeFamily,
            this->queueFamilyIndices.presentFamily
        };

        float queuePriority = 1.0f;                                                                    // ť�� �켱������ �����մϴ�.
        
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;                        // ����ü Ÿ���� �����մϴ�.
            queueCreateInfo.queueFamilyIndex = queueFamily;                                            // �׷��Ƚ� ť �йи� �ε����� �����մϴ�.
            queueCreateInfo.queueCount = 1;                                                            // ť�� ������ �����մϴ�.
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // �� ��ġ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkDeviceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;                            // ����ü Ÿ���� �����մϴ�.
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());   // ť ���� ������ ������ �����մϴ�.
        createInfo.pQueueCreateInfos = queueCreateInfos.data();                             // ť ���� ���� �����͸� �����մϴ�.
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());  // Ȱ��ȭ�� Ȯ�� ������ �����մϴ�.
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();                       // Ȱ��ȭ�� Ȯ�� ����� �����մϴ�.
        createInfo.pEnabledFeatures = &this->features;                                      // ���� ��ġ ��� �����͸� �����մϴ�.

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // �� ��ġ�� �����մϴ�.
        result = vkCreateDevice(this->VKphysicalDevice, &createInfo, nullptr, &this->VKdevice);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!"); // �� ��ġ ���� ���� �� ���ܸ� �߻���ŵ�ϴ�.
        }

        // �� ����̽����� �׷��� ť �ڵ��� �����ɴϴ�.
        vkGetDeviceQueue(this->VKdevice, this->queueFamilyIndices.graphicsAndComputeFamily, 0, &this->graphicsVKQueue);
        
        // �� ����̽����� ���������̼� ť �ڵ��� �����ɴϴ�.
        vkGetDeviceQueue(this->VKdevice, this->queueFamilyIndices.presentFamily, 0, &this->presentVKQueue);

        return result;
    }

    VKDevice_::~VKDevice_()
    {
    }
}