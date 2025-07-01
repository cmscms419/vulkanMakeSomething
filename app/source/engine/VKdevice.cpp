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

        vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);       // 물리 디바이스 속성 가져오기
        vkGetPhysicalDeviceFeatures(this->physicalDevice, &features);           // 물리 디바이스 기능 가져오기
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties); // 메모리 속성 가져오기

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

        _PRINT_TO_CONSOLE_("Select Device\n");
        _PRINT_TO_CONSOLE_("Select DeviceProperties.deviceType: %d\n", properties.deviceType);
        _PRINT_TO_CONSOLE_("Select Device Name: %s\n", properties.deviceName);
        
        helper::getDeviceExtensionSupport(physicalDevice, &this->supportedExtensions);
    }

    VkResult VKDevice_::createLogicalDevice()
    {
        // 1. 물리 장치에서 큐 패밀리 인덱스를 찾습니다. -> 전 단계에서 이미 찾았습니다.
        // 2. 큐 생성 정보 구조체를 초기화합니다. -> 이미 찾았습니다.
        // 3. 물리 장치 기능 구조체를 초기화합니다.
        // 4. 논리 장치 생성 정보 구조체를 초기화합니다.
        // 5. 논리 장치를 생성합니다.
        VkResult result = VK_SUCCESS;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::set<uint32_t> uniqueQueueFamilies = {
            this->queueFamilyIndices.graphicsAndComputeFamily,
            this->queueFamilyIndices.presentFamily
        };

        cFloat queuePriority = 1.0f;                                                                    // 큐의 우선순위를 설정합니다.
        
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;                        // 구조체 타입을 지정합니다.
            queueCreateInfo.queueFamilyIndex = queueFamily;                                            // 그래픽스 큐 패밀리 인덱스를 설정합니다.
            queueCreateInfo.queueCount = 1;                                                            // 큐의 개수를 설정합니다.
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 논리 장치 생성 정보 구조체를 초기화합니다.
        VkDeviceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;                            // 구조체 타입을 지정합니다.
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());   // 큐 생성 정보의 개수를 설정합니다.
        createInfo.pQueueCreateInfos = queueCreateInfos.data();                             // 큐 생성 정보 포인터를 설정합니다.
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());  // 활성화할 확장 개수를 설정합니다.
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();                       // 활성화할 확장 목록을 설정합니다.
        createInfo.pEnabledFeatures = &this->features;                                      // 물리 장치 기능 포인터를 설정합니다.

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // 논리 장치를 생성합니다.
        result = vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicaldevice);

        if (result != VK_SUCCESS) {
            return result;
        }

        // 논리 디바이스에서 그래픽 큐 핸들을 가져옵니다.
        vkGetDeviceQueue(this->logicaldevice, this->queueFamilyIndices.graphicsAndComputeFamily, 0, &this->graphicsVKQueue);
        
        // 논리 디바이스에서 프레젠테이션 큐 핸들을 가져옵니다.
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

    // 커맨드 버퍼를 플러시하고 큐에 제출합니다.
    // 커맨드 버퍼(command buffer)를 큐(queue)에 제출할 때,
    // 해당 커맨드 버퍼가 할당된 커맨드 풀(command pool)과 
    // 동일한 큐 패밀리 인덱스(queue family index)를 가진 큐에만 제출해야 한다는 의미입니다.
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