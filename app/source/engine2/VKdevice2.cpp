#include "VKdevice2.h"

using namespace vkengine::Log;

namespace vkengine {

    using namespace helper;

    VKdeviceHandler2::VKdeviceHandler2(VkPhysicalDevice physicalDevice, QueueFamilyIndices2 indice)
    {
        assert(physicalDevice);
        assert(indice.isComplete());

        this->physicalDevice = physicalDevice;
        this->queueFamilyIndices = indice;

        vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);       // 물리 디바이스 속성 가져오기
        vkGetPhysicalDeviceFeatures(this->physicalDevice, &features);           // 물리 디바이스 기능 가져오기
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties); // 메모리 속성 가져오기

        uint32_t supportedApiVersion = properties.apiVersion;


        if (supportedApiVersion >= VK_API_VERSION_1_3) {
            // Vulkan 1.3 이상을 지원하는 디바이스 처리
            // 예: 새로운 기능 활성화

        }
        else if (supportedApiVersion >= VK_API_VERSION_1_2) {
            // Vulkan 1.2 이상을 지원하는 디바이스 처리
            // 예: 새로운 기능 활성화

        }
        else if (supportedApiVersion >= VK_API_VERSION_1_1) {
            // Vulkan 1.1을 지원하는 디바이스 처리
        }
        else {
            // Vulkan 1.0을 지원하는 디바이스 처리
        }

        PRINT_TO_LOGGER("Select Device\n");
        PRINT_TO_LOGGER("Select DeviceProperties.deviceType: %d\n", properties.deviceType);
        PRINT_TO_LOGGER("Select Device Name: %s\n", properties.deviceName);

        helper::getDeviceExtensionSupport(physicalDevice, &this->supportedExtensions);
    }

    VKdeviceHandler2::~VKdeviceHandler2()
    {
        cleanup();
    }

    void VKdeviceHandler2::printPysicaldeviceProperties() const
    {
        PRINT_TO_LOGGER("Selected %s (%s)\n", properties.deviceName,
            helper::getPhysicalDeviceTypeString(properties.deviceType).c_str());
        PRINT_TO_LOGGER("  nonCoherentAtomSize: %llu\n", properties.limits.nonCoherentAtomSize);
        PRINT_TO_LOGGER("  Max UBO size: %u KBytes\n", properties.limits.maxUniformBufferRange / 1024);
        PRINT_TO_LOGGER("  Max SSBO size: %u KBytes\n", properties.limits.maxStorageBufferRange / 1024);
        PRINT_TO_LOGGER("  UBO offset alignment: %llu\n",
            properties.limits.minUniformBufferOffsetAlignment);
        PRINT_TO_LOGGER("  SSBO offset alignment: %llu\n",
            properties.limits.minStorageBufferOffsetAlignment);

        PRINT_TO_LOGGER("\nDevice Features:\n");
        PRINT_TO_LOGGER("  geometryShader: %s\n", features.geometryShader ? "YES" : "NO");
        PRINT_TO_LOGGER("  tessellationShader: %s\n", features.tessellationShader ? "YES" : "NO");

        PRINT_TO_LOGGER("\nDevice Memory Properties:\n");
        PRINT_TO_LOGGER("  Memory Type Count: %d\n", memoryProperties.memoryTypeCount);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
            const auto& memType = memoryProperties.memoryTypes[i];
            cString propFlags;
            if (memType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                propFlags += "DEVICE_LOCAL ";
            if (memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                propFlags += "HOST_VISIBLE ";
            if (memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                propFlags += "HOST_COHERENT ";
            if (memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
                propFlags += "HOST_CACHED ";
            if (memType.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
                propFlags += "LAZILY_ALLOCATED ";
            if (memType.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
                propFlags += "PROTECTED ";
            if (propFlags.empty())
                propFlags = "NONE ";

            PRINT_TO_LOGGER("    Memory Type %d: heap %d, flags: %s\n", i, memType.heapIndex, propFlags.c_str());
        }

        PRINT_TO_LOGGER("  Memory Heap Count: %d\n", memoryProperties.memoryHeapCount);
        for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
            const auto& memHeap = memoryProperties.memoryHeaps[i];
            cString propFlags;
            if (memHeap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                propFlags += "DEVICE_LOCAL ";
            if (memHeap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT)
                propFlags += "MULTI_INSTANCE ";
            if (propFlags.empty())
                propFlags = "NONE ";
            PRINT_TO_LOGGER("    Memory Heap %d: size %llu MBytes, flags: %s\n", i,
                memHeap.size / 1024 / 1024, propFlags.c_str());
        }

    }

    cBool VKdeviceHandler2::createLogicalDevice(cBool useSwapChain)
    {
        // 1. 물리 장치에서 큐 패밀리 인덱스를 찾습니다. -> 전 단계에서 이미 찾았습니다.
        // 2. 큐 생성 정보 구조체를 초기화합니다. -> 이미 찾았습니다.
        // 3. 물리 장치 기능 구조체를 초기화합니다.
        // 4. 논리 장치 생성 정보 구조체를 초기화합니다.
        // 5. 논리 장치를 생성합니다.
        cBool result = true;

        const VkQueueFlags requestedQueueTypes = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;

        VkPhysicalDeviceVulkan13Features enabledFeatures13{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        enabledFeatures13.dynamicRendering = VK_TRUE;
        enabledFeatures13.synchronization2 = VK_TRUE;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

        const float defaultQueuePriority(0.0f);

        if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.grapicFamily;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
        else {
            queueFamilyIndices.grapicFamily = 0;
        }

        if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
            if (queueFamilyIndices.computerFamily != queueFamilyIndices.grapicFamily) {
                VkDeviceQueueCreateInfo queueInfo{};
                queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfo.queueFamilyIndex = queueFamilyIndices.computerFamily;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        }
        else {

            queueFamilyIndices.computerFamily = queueFamilyIndices.grapicFamily;
        }

        if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
            if ((queueFamilyIndices.transferFamily != queueFamilyIndices.grapicFamily) &&
                (queueFamilyIndices.transferFamily != queueFamilyIndices.computerFamily)) {
                VkDeviceQueueCreateInfo queueInfo{};
                queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfo.queueFamilyIndex = queueFamilyIndices.transferFamily;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        }
        else {
            queueFamilyIndices.transferFamily = queueFamilyIndices.grapicFamily;
        }

        std::vector<const char*> deviceExtensions(enabledDeviceExtensions);

        if (useSwapChain) {
            deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }

        enabledFeatures.samplerAnisotropy = features.samplerAnisotropy;
        enabledFeatures.depthClamp = features.depthClamp;
        enabledFeatures.depthBiasClamp = features.depthBiasClamp;

        // 논리 장치 생성 정보 구조체를 초기화합니다.
        VkDeviceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;                            // 구조체 타입을 지정합니다.
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());   // 큐 생성 정보의 개수를 설정합니다.
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());  // 활성화할 확장 개수를 설정합니다.
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();                       // 활성화할 확장 목록을 설정합니다.

        VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
        physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physicalDeviceFeatures2.features = enabledFeatures;
        physicalDeviceFeatures2.pNext = &enabledFeatures13;
        createInfo.pEnabledFeatures = nullptr;
        createInfo.pNext = &physicalDeviceFeatures2;

        _VK_CHECK_RESULT_(vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicaldevice));

        return result;
    }

    cBool VKdeviceHandler2::createCommandPools()
    {
        cBool result = true;

        if (this->graphicsCommandPool == VK_NULL_HANDLE)
        {
            VkCommandPoolCreateInfo poolInfo = helper::commandPoolCreateInfo(queueFamilyIndices.grapicFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

            _VK_CHECK_RESULT_(vkCreateCommandPool(this->logicaldevice, &poolInfo, nullptr, &this->graphicsCommandPool));
        }

        // 그래픽스와 컴퓨트가 같은 패밀리면 커맨드 풀도 하나만 생성
        if (queueFamilyIndices.grapicFamily != queueFamilyIndices.computerFamily)
        {
            VkCommandPoolCreateInfo poolInfo = helper::commandPoolCreateInfo(queueFamilyIndices.computerFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            _VK_CHECK_RESULT_(vkCreateCommandPool(this->logicaldevice, &poolInfo, nullptr, &this->computeCommandPool));
        }
        else
        {
            this->computeCommandPool = this->graphicsCommandPool;
        }

        if (this->queueFamilyIndices.transferFamily != queueFamilyIndices.grapicFamily &&
            this->queueFamilyIndices.transferFamily != queueFamilyIndices.computerFamily)
        {
            VkCommandPoolCreateInfo poolInfo = helper::commandPoolCreateInfo(queueFamilyIndices.transferFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            _VK_CHECK_RESULT_(vkCreateCommandPool(this->logicaldevice, &poolInfo, nullptr, &this->transferCommandPool));
        }
        else if (this->queueFamilyIndices.transferFamily == this->queueFamilyIndices.computerFamily)
        {
            this->transferCommandPool = this->computeCommandPool;
        }
        else
        {
            this->transferCommandPool = this->graphicsCommandPool;
        }

        return result;
    }

    cBool VKdeviceHandler2::createQueues()
    {

        if (this->queueFamilyIndices.grapicFamily == cUint32_t(-1) ||
            this->queueFamilyIndices.computerFamily == cUint32_t(-1) ||
            this->queueFamilyIndices.transferFamily == cUint32_t(-1))
        {
            PRINT_TO_LOGGER("Error:: createQueues:: invalid queue family index");
            return false;
        }
        // 그래픽 큐 핸들을 가져옵니다.
        vkGetDeviceQueue(this->logicaldevice, this->queueFamilyIndices.grapicFamily, 0, &this->graphicsVKQueue);

        // 컴퓨터 큐 핸들을 가져옵니다.
        if (queueFamilyIndices.computerFamily != queueFamilyIndices.grapicFamily &&
            queueFamilyIndices.computerFamily != uint32_t(-1)) {
            vkGetDeviceQueue(this->logicaldevice, queueFamilyIndices.computerFamily, 0, &this->computerVKQueue);
        }
        else {
            this->computerVKQueue = this->graphicsVKQueue;
        }

        // 트랜스퍼 큐 핸들을 가져옵니다.
        if (queueFamilyIndices.transferFamily != queueFamilyIndices.grapicFamily &&
            queueFamilyIndices.transferFamily != queueFamilyIndices.computerFamily &&
            queueFamilyIndices.transferFamily != uint32_t(-1)) {
            vkGetDeviceQueue(this->logicaldevice, queueFamilyIndices.transferFamily, 0, &this->transferVKQueue);
        }
        else if (queueFamilyIndices.transferFamily == queueFamilyIndices.computerFamily) {
            this->transferVKQueue = this->computerVKQueue;
        }
        else {
            this->transferVKQueue = this->graphicsVKQueue;
        }

        if (this->graphicsVKQueue == VK_NULL_HANDLE ||
            this->computerVKQueue == VK_NULL_HANDLE ||
            this->transferVKQueue == VK_NULL_HANDLE)
        {
            PRINT_TO_LOGGER("Error:: createQueues:: failed to get queue handle");
            return false;
        }


        return true;
    }

    const VkShaderModule VKdeviceHandler2::createShaderModule(const std::string& path) const
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

    void VKdeviceHandler2::cleanup()
    {

        std::set<VkCommandPool> uniquePools;

        if (graphicsCommandPool != VK_NULL_HANDLE) {
            uniquePools.insert(graphicsCommandPool);
        }
        if (computeCommandPool != VK_NULL_HANDLE) {
            uniquePools.insert(computeCommandPool);
        }
        if (transferCommandPool != VK_NULL_HANDLE) {
            uniquePools.insert(transferCommandPool);
        }

        for (VkCommandPool pool : uniquePools) {
            vkDestroyCommandPool(logicaldevice, pool, nullptr);
        }

        graphicsCommandPool = VK_NULL_HANDLE;
        computeCommandPool = VK_NULL_HANDLE;
        transferCommandPool = VK_NULL_HANDLE;

        if (logicaldevice != VK_NULL_HANDLE)
        {
            vkDestroyDevice(logicaldevice, nullptr);
            logicaldevice = VK_NULL_HANDLE;
        }


    }
}