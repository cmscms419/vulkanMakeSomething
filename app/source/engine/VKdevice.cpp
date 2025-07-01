#include "VKdevice.h"
#include "helper.h"

namespace vkengine {

    using namespace helper;

    VKDevice_::VKDevice_(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice)
    {
        assert(physicalDevice);
        assert(indice.isComplete());

        this->physicalDevice = physicalDevice;
        this->queueFamilyIndices = indice;

        vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);       // ���� ����̽� �Ӽ� ��������
        vkGetPhysicalDeviceFeatures(this->physicalDevice, &features);           // ���� ����̽� ��� ��������
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties); // �޸� �Ӽ� ��������

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

        _PRINT_TO_CONSOLE_("Select Device\n");
        _PRINT_TO_CONSOLE_("Select DeviceProperties.deviceType: %d\n", properties.deviceType);
        _PRINT_TO_CONSOLE_("Select Device Name: %s\n", properties.deviceName);
        
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

        cFloat queuePriority = 1.0f;                                                                    // ť�� �켱������ �����մϴ�.
        
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
        result = vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicaldevice);

        if (result != VK_SUCCESS) {
            return result;
        }

        // �� ����̽����� �׷��� ť �ڵ��� �����ɴϴ�.
        vkGetDeviceQueue(this->logicaldevice, this->queueFamilyIndices.graphicsAndComputeFamily, 0, &this->graphicsVKQueue);
        
        // �� ����̽����� ���������̼� ť �ڵ��� �����ɴϴ�.
        vkGetDeviceQueue(this->logicaldevice, this->queueFamilyIndices.presentFamily, 0, &this->presentVKQueue);

        return result;
    }

    void VKDevice_::createimageview(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const
    {
        VkImageCreateInfo imageInfo{};

        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = { width, height, 1 };
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0; // Optional

        _VK_CHECK_RESULT_(vkCreateImage(logicaldevice, &imageInfo, nullptr, &image));

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logicaldevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = helper::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        _VK_CHECK_RESULT_(vkAllocateMemory(logicaldevice, &allocInfo, nullptr, &imageMemory));
        _VK_CHECK_RESULT_(vkBindImageMemory(logicaldevice, image, imageMemory, 0));
    }

    const VkShaderModule VKDevice_::createShaderModule(const std::string& path) const
    {
        auto shaderCode = helper::readFile(path);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        VkShaderModule shaderModule{ VK_NULL_HANDLE };
        _VK_CHECK_RESULT_(vkCreateShaderModule(this->logicaldevice, &createInfo, nullptr, &shaderModule));

        return shaderModule;
    }

    // Ŀ�ǵ� ���۸� �÷����ϰ� ť�� �����մϴ�.
    // Ŀ�ǵ� ����(command buffer)�� ť(queue)�� ������ ��,
    // �ش� Ŀ�ǵ� ���۰� �Ҵ�� Ŀ�ǵ� Ǯ(command pool)�� 
    // ������ ť �йи� �ε���(queue family index)�� ���� ť���� �����ؾ� �Ѵٴ� �ǹ��Դϴ�.
    void VKDevice_::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
    {
        if (commandBuffer == VK_NULL_HANDLE)
        {
            return;
        }

        _VK_CHECK_RESULT_(vkEndCommandBuffer(commandBuffer));
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkFenceCreateInfo fenceInfo = helper::fenceCreateInfo(0);
        VkFence fence;
        _VK_CHECK_RESULT_(vkCreateFence(this->logicaldevice, &fenceInfo, nullptr, &fence));
        // Submit to the queue
        _VK_CHECK_RESULT_(vkQueueSubmit(queue, 1, &submitInfo, fence));
        // Wait for the fence to signal that command buffer has finished executing
        _VK_CHECK_RESULT_(vkWaitForFences(this->logicaldevice, 1, &fence, VK_TRUE, 100000000000));
        vkDestroyFence(this->logicaldevice, fence, nullptr);

        if (free)
        {
            vkFreeCommandBuffers(this->logicaldevice, pool, 1, &commandBuffer);
        }
    }

    void VKDevice_::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
    {
        return flushCommandBuffer(commandBuffer, queue, commandPool, free);
    }

    VkCommandBuffer VKDevice_::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
    {
        VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(pool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        VkCommandBuffer cmdBuffer{};
        _VK_CHECK_RESULT_(vkAllocateCommandBuffers(this->logicaldevice, &allocInfo, &cmdBuffer));

        if (begin)
        {
            FrameData* frameData{};
            VkCommandBufferBeginInfo beginInfo = frameData->commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            _VK_CHECK_RESULT_(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
        }

        return cmdBuffer;
    }

    VkCommandBuffer VKDevice_::createCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        return createCommandBuffer(level, commandPool, begin);
    }


    void VKDevice_::cleanup() const
    {
        vkDestroyCommandPool(logicaldevice, commandPool, nullptr);
        vkDestroyDevice(logicaldevice, nullptr);
    }
}