#include "DeviceManager2.h"

#include "data.h"
#include "helper.h"
#include "log.h"


namespace vkengine {


    DeviceManager2::DeviceManager2(VkPhysicalDevice physicalDevice, QueueFamilyIndices2 indice)
    {
        logicaldevice = VK_NULL_HANDLE;
        properties = {};
        features = {};
        enabledFeatures = {};
        memoryProperties = {};
        queueFamilyProperties = {};
        supportedExtensions = {};
        enabledDeviceExtensions = {};
        graphicsCommandPool = VK_NULL_HANDLE;
        computeCommandPool = VK_NULL_HANDLE;
        transferCommandPool = VK_NULL_HANDLE;
        graphicsVKQueue = VK_NULL_HANDLE;
        computeVKQueue = VK_NULL_HANDLE;
        transferVKQueue = VK_NULL_HANDLE;

        assert(physicalDevice);
        assert(indice.isComplete());

        this->physicalDevice = physicalDevice;
        this->queueFamilyIndices = indice;

        vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);       // 물리 디바이스 속성 가져오기
        vkGetPhysicalDeviceFeatures(this->physicalDevice, &features);           // 물리 디바이스 기능 가져오기
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties); // 메모리 속성 가져오기

        cUint32_t supportedApiVersion = properties.apiVersion;

        // queue family properties 가져오기
        cUint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount, nullptr);
        if (queueFamilyCount <= 0)
        {
            Log::exitToLogger("Failed to get queue family properties");
        }
        this->queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount,
                                                 this->queueFamilyProperties.data());


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

        Log::printToLogger("Select Device");
        Log::printToLogger("Select DeviceProperties.deviceType: %d", properties.deviceType);
        Log::printToLogger("Select Device Name: %s", properties.deviceName);

        helper::device::getDeviceExtensionSupport(physicalDevice, &this->supportedExtensions);
    }

    DeviceManager2::~DeviceManager2()
    {
        cleanup();
    }

    void DeviceManager2::printPhysicalDeviceProperties() const
    {
        Log::printToLogger("Selected %s (%s)", properties.deviceName,
                        helper::device::getPhysicalDeviceTypeString(properties.deviceType).c_str());
        Log::printToLogger("  nonCoherentAtomSize: %llu", properties.limits.nonCoherentAtomSize);
        Log::printToLogger("  Max UBO size: %u KBytes", properties.limits.maxUniformBufferRange / 1024);
        Log::printToLogger("  Max SSBO size: %u KBytes", properties.limits.maxStorageBufferRange / 1024);
        Log::printToLogger("  UBO offset alignment: %llu",
                        properties.limits.minUniformBufferOffsetAlignment);
        Log::printToLogger("  SSBO offset alignment: %llu",
                        properties.limits.minStorageBufferOffsetAlignment);

        Log::printToLogger("Device Features:");
        Log::printToLogger("  geometryShader: %s", features.geometryShader ? "YES" : "NO");
        Log::printToLogger("  tessellationShader: %s", features.tessellationShader ? "YES" : "NO");

        Log::printToLogger("Device Memory Properties:");
        Log::printToLogger("  Memory Type Count: %d", memoryProperties.memoryTypeCount);

        for (cUint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
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

            Log::printToLogger("    Memory Type %d: heap %d, flags: %s", i, memType.heapIndex, propFlags.c_str());
        }

        Log::printToLogger("  Memory Heap Count: %d", memoryProperties.memoryHeapCount);
        for (cUint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
            const auto& memHeap = memoryProperties.memoryHeaps[i];
            cString propFlags;
            if (memHeap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                propFlags += "DEVICE_LOCAL ";
            if (memHeap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT)
                propFlags += "MULTI_INSTANCE ";
            if (propFlags.empty())
                propFlags = "NONE ";
            Log::printToLogger("    Memory Heap %d: size %llu MBytes, flags: %s", i,
                            memHeap.size / 1024 / 1024, propFlags.c_str());
        }

    }

    cBool DeviceManager2::createLogicalDevice(cBool useSwapChain)
    {
        // 1. 물리 장치에서 큐 패밀리 인덱스를 찾습니다. -> 전 단계에서 이미 찾았습니다.
        // 2. 큐 생성 정보 구조체를 초기화합니다. -> 이미 찾았습니다.
        // 3. 물리 장치 기능 구조체를 초기화합니다.
        // 4. 논리 장치 생성 정보 구조체를 초기화합니다.
        // 5. 논리 장치를 생성합니다.
        cBool result = true;

        const VkQueueFlags requestedQueueTypes = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;

        // Vulkan Query descriptor indexing features
        VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
        descriptorIndexingFeatures.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &descriptorIndexingFeatures;

        vkGetPhysicalDeviceFeatures2(this->physicalDevice, &deviceFeatures2);

        // Check required descriptor indexing features
        if (!descriptorIndexingFeatures.descriptorBindingPartiallyBound ||
            !descriptorIndexingFeatures.runtimeDescriptorArray ||
            !descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount)
        {
            Log::exitToLogger(
                "GPU does not support required descriptor indexing features for bindless textures:"
                "  - descriptorBindingPartiallyBound: %s"
                "  - runtimeDescriptorArray: %s"
                "  - descriptorBindingVariableDescriptorCount: %s",
                descriptorIndexingFeatures.descriptorBindingPartiallyBound ? "YES" : "NO",
                descriptorIndexingFeatures.runtimeDescriptorArray ? "YES" : "NO",
                descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount ? "YES" : "NO");
        }

        Log::printToLogger("Descriptor indexing features supported:");
        Log::printToLogger("  descriptorBindingPartiallyBound: YES");
        Log::printToLogger("  runtimeDescriptorArray: YES");
        Log::printToLogger("  descriptorBindingVariableDescriptorCount: YES");

        VkPhysicalDeviceVulkan13Features enabledFeatures13{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        enabledFeatures13.dynamicRendering = VK_TRUE;
        enabledFeatures13.synchronization2 = VK_TRUE;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

        const float defaultQueuePriority(0.0f);

        if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.graphicFamily;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
        else {
            queueFamilyIndices.graphicFamily = 0;
        }

        if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
            if (queueFamilyIndices.computeFamily != queueFamilyIndices.graphicFamily) {
                VkDeviceQueueCreateInfo queueInfo{};
                queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfo.queueFamilyIndex = queueFamilyIndices.computeFamily;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        }
        else {

            queueFamilyIndices.computeFamily = queueFamilyIndices.graphicFamily;
        }

        if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
            if ((queueFamilyIndices.transferFamily != queueFamilyIndices.graphicFamily) &&
                (queueFamilyIndices.transferFamily != queueFamilyIndices.computeFamily)) {
                VkDeviceQueueCreateInfo queueInfo{};
                queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfo.queueFamilyIndex = queueFamilyIndices.transferFamily;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        }
        else {
            queueFamilyIndices.transferFamily = queueFamilyIndices.graphicFamily;
        }

        std::vector<const cChar*> deviceExtensions;

        if (useSwapChain) {
            deviceExtensions = coreDeviceExtensions;
        }

        enabledFeatures.samplerAnisotropy = features.samplerAnisotropy;
        enabledFeatures.depthClamp = features.depthClamp;
        enabledFeatures.depthBiasClamp = features.depthBiasClamp;

        // Enable descriptor indexing features
        descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;

        // 논리 장치 생성 정보 구조체를 초기화합니다.
        VkDeviceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;                            // 구조체 타입을 지정합니다.
        createInfo.queueCreateInfoCount = static_cast<cUint32_t>(queueCreateInfos.size());   // 큐 생성 정보의 개수를 설정합니다.
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<cUint32_t>(deviceExtensions.size());  // 활성화할 확장 개수를 설정합니다.
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();                       // 활성화할 확장 목록을 설정합니다.

        VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
        physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physicalDeviceFeatures2.features = enabledFeatures;
        physicalDeviceFeatures2.pNext = &enabledFeatures13;
        createInfo.pEnabledFeatures = nullptr;
        createInfo.pNext = &physicalDeviceFeatures2;

        // chain descriptorindexing featrues
        enabledFeatures13.pNext = &descriptorIndexingFeatures;

        _VK_CHECK_RESULT_(vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicaldevice));

        return result;
    }

    cBool DeviceManager2::createCommandPools()
    {
        cBool result = true;

        if (this->graphicsCommandPool == VK_NULL_HANDLE)
        {
            VkCommandPoolCreateInfo poolInfo = helper::device::makeCommandPoolCreateInfo(queueFamilyIndices.graphicFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

            _VK_CHECK_RESULT_(vkCreateCommandPool(this->logicaldevice, &poolInfo, nullptr, &this->graphicsCommandPool));
        }

        // 그래픽스와 컴퓨트가 같은 패밀리면 커맨드 풀도 하나만 생성
        if (queueFamilyIndices.graphicFamily != queueFamilyIndices.computeFamily)
        {
            VkCommandPoolCreateInfo poolInfo = helper::device::makeCommandPoolCreateInfo(queueFamilyIndices.computeFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            _VK_CHECK_RESULT_(vkCreateCommandPool(this->logicaldevice, &poolInfo, nullptr, &this->computeCommandPool));
        }
        else
        {
            this->computeCommandPool = this->graphicsCommandPool;
        }

        if (this->queueFamilyIndices.transferFamily != queueFamilyIndices.graphicFamily &&
            this->queueFamilyIndices.transferFamily != queueFamilyIndices.computeFamily)
        {
            VkCommandPoolCreateInfo poolInfo = helper::device::makeCommandPoolCreateInfo(queueFamilyIndices.transferFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
            _VK_CHECK_RESULT_(vkCreateCommandPool(this->logicaldevice, &poolInfo, nullptr, &this->transferCommandPool));
        }
        else if (this->queueFamilyIndices.transferFamily == this->queueFamilyIndices.computeFamily)
        {
            this->transferCommandPool = this->computeCommandPool;
        }
        else
        {
            this->transferCommandPool = this->graphicsCommandPool;
        }

        return result;
    }

    cBool DeviceManager2::createQueues()
    {

        if (this->queueFamilyIndices.graphicFamily == cUint32_t(-1) ||
            this->queueFamilyIndices.computeFamily == cUint32_t(-1) ||
            this->queueFamilyIndices.transferFamily == cUint32_t(-1))
        {
            Log::printToLogger("Error:: createQueues:: invalid queue family index");
            return false;
        }
        // 그래픽 큐 핸들을 가져옵니다.
        vkGetDeviceQueue(this->logicaldevice, this->queueFamilyIndices.graphicFamily, 0, &this->graphicsVKQueue);

        // 컴퓨터 큐 핸들을 가져옵니다.
        if (queueFamilyIndices.computeFamily != queueFamilyIndices.graphicFamily &&
            queueFamilyIndices.computeFamily != cUint32_t(-1)) {
            vkGetDeviceQueue(this->logicaldevice, queueFamilyIndices.computeFamily, 0, &this->computeVKQueue);
        }
        else {
            this->computeVKQueue = this->graphicsVKQueue;
        }

        // 트랜스퍼 큐 핸들을 가져옵니다.
        if (queueFamilyIndices.transferFamily != queueFamilyIndices.graphicFamily &&
            queueFamilyIndices.transferFamily != queueFamilyIndices.computeFamily &&
            queueFamilyIndices.transferFamily != cUint32_t(-1)) {
            vkGetDeviceQueue(this->logicaldevice, queueFamilyIndices.transferFamily, 0, &this->transferVKQueue);
        }
        else if (queueFamilyIndices.transferFamily == queueFamilyIndices.computeFamily) {
            this->transferVKQueue = this->computeVKQueue;
        }
        else {
            this->transferVKQueue = this->graphicsVKQueue;
        }

        if (this->graphicsVKQueue == VK_NULL_HANDLE ||
            this->computeVKQueue == VK_NULL_HANDLE ||
            this->transferVKQueue == VK_NULL_HANDLE)
        {
            Log::printToLogger("Error:: createQueues:: failed to get queue handle");
            return false;
        }


        return true;
    }

    const VkShaderModule DeviceManager2::createShaderModule(const std::string& path) const
    {
        auto shaderCode = helper::file::readFile(path);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        createInfo.pCode = reinterpret_cast<const cUint32_t*>(shaderCode.data());

        VkShaderModule shaderModule{ VK_NULL_HANDLE };
        _VK_CHECK_RESULT_(vkCreateShaderModule(this->logicaldevice, &createInfo, nullptr, &shaderModule));

        return shaderModule;
    }

    void DeviceManager2::cleanup()
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