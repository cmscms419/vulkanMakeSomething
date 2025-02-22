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

        helper::getDeviceExtensionSupport(physicalDevice, &this->supportedExtensions);
    }

    VkResult VKDevice_::createLogicalDevice()
    {
        // 1. ���� ��ġ���� ť �йи� �ε����� ã���ϴ�. -> �� �ܰ迡�� �̹� ã�ҽ��ϴ�.
        // 2. ť ���� ���� ����ü�� �ʱ�ȭ�մϴ�. -> �̹� ã�ҽ��ϴ�.
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
            return result;
        }

        // �� ����̽����� �׷��� ť �ڵ��� �����ɴϴ�.
        vkGetDeviceQueue(this->VKdevice, this->queueFamilyIndices.graphicsAndComputeFamily, 0, &this->graphicsVKQueue);
        
        // �� ����̽����� ���������̼� ť �ڵ��� �����ɴϴ�.
        vkGetDeviceQueue(this->VKdevice, this->queueFamilyIndices.presentFamily, 0, &this->presentVKQueue);

        return result;
    }

    void VKDevice_::createimageview(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo{};

        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0; // Optional

        if (vkCreateImage(VKdevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(VKdevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = helper::findMemoryType(VKphysicalDevice, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(VKdevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(VKdevice, image, imageMemory, 0);
    }
    void VKDevice_::cleanup()
    {
        vkDestroyCommandPool(VKdevice, VKcommandPool, nullptr);
        vkDestroyDevice(VKdevice, nullptr);
    }
}