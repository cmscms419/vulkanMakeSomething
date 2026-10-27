#include "Context.h"

#include <map>

#include "data.h"
#include "log.h"
#include "debug.h"

#include "helper.h"


namespace vkengine {
    Context::Context(const std::vector<const char*>& requiredInstanceExtensions, cBool useSwapchain)
        : descriptorManager2(VKdevice.logicaldevice)
    {
        VKinstance = VK_NULL_HANDLE;
        VKpipelineCache = VK_NULL_HANDLE;
        VKdepthStencil = {};
        VKdebugUtilsMessenger = VK_NULL_HANDLE;  // 디버그 메신저 -> 디버깅을 위한 메신저
        useDepthStencil = false;

        this->createInstance(requiredInstanceExtensions);

        if (enableValidationLayers)
        {
            this->createDebugMessenger();
        }

        this->createPhysicalDevice();
        this->createLogicalDevice(useSwapchain);
        this->createCommandPools();
        this->createQueues();
        this->createPipelineCache();
        this->createDepthStencilFormat();
    }

    Context::~Context() {
        cleanup();
    }

    void Context::cleanup() {

        if (this->VKdevice.logicaldevice != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(this->VKdevice.logicaldevice);
        }

        descriptorManager2.cleanup();

        if (VKpipelineCache != VK_NULL_HANDLE) {
            vkDestroyPipelineCache(VKdevice.logicaldevice, VKpipelineCache, nullptr);
            VKpipelineCache = VK_NULL_HANDLE;
        }

        this->VKdepthStencil.cleanup(this->VKdevice.logicaldevice);
        VKdevice.cleanup();

        if (enableValidationLayers) {
            debug::DestroyDebugUtilsMessengerEXT(this->VKinstance, this->VKdebugUtilsMessenger, nullptr);
        }

        if (VKinstance != VK_NULL_HANDLE) {
            vkDestroyInstance(VKinstance, nullptr);
            VKinstance = VK_NULL_HANDLE;
        }

    }
    
    cBool Context::createInstance(std::vector<const char*> requiredInstanceExtensions)
    {
        cBool check = false;

        // debug 가능한지 확인
        for (const cChar* layerName : validationLayers) {
            if (enableValidationLayers && !checkValidationLayerSupport(layerName)) {
                Log::printToLogger("validation layer not supported: %s", layerName);
                return check;
            }
        }

        std::vector<cString> supportedInstanceExtensions;

        cUint32_t extCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
        if (extCount > 0) {
            std::vector<VkExtensionProperties> extensions(extCount);
            if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) ==
                VK_SUCCESS) {
                for (VkExtensionProperties& extension : extensions) {
                    supportedInstanceExtensions.push_back(extension.extensionName);
                }
            }
        }

        // print instanceExtensions
        Log::printToLogger("Supported Instance Extensions: ");
        for (const cString& extension : supportedInstanceExtensions) {
            Log::printToLogger("  %s", extension.c_str());
        }

        // MoltenVK on macOS/iOS supported
        const char* portabilityExtension = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
        bool portabilityAlreadyAdded = false;
        for (const char* ext : requiredInstanceExtensions) {
            if (strcmp(ext, portabilityExtension) == 0) {
                portabilityAlreadyAdded = true;
                break;
            }
        }

        // Add VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, "VK_KHR_xlib_surface" and
        // "VK_KHR_wayland_surface" for MacOS, Linux support if they are available in
        // supportedInstanceExtensions

        bool portabilitySupported =
            find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(),
                portabilityExtension) != supportedInstanceExtensions.end();

        // if (!portabilityAlreadyAdded && portabilitySupported) {
        //     requiredInstanceExtensions.push_back(portabilityExtension);
        //     portabilityAlreadyAdded = true;
        // }

        // Validate all required extensions are supported
        for (const char* requiredExtension : requiredInstanceExtensions) {
            if (find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(),
                requiredExtension) == supportedInstanceExtensions.end()) {
                _EXIT_WITH_MESSAGE_("Required instance extension \"%s\" is not supported",
                    requiredExtension);
            }
        }

        // VkApplicationInfo 구조체는 애플리케이션에 대한 정보를 Vulkan에게 제공하기 위해 사용됩니다.
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;     // 구조체 타입을 지정합니다.
        appInfo.pApplicationName = "Vulkan egine";              // 애플리케이션 이름을 지정합니다.
        appInfo.pEngineName = "vulkanEngine";                   // 엔진 이름을 지정합니다.
        appInfo.apiVersion = VK_API_VERSION_1_3;                // 사용할 Vulkan API 버전을 지정합니다.
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);  // 애플리케이션 버전을 지정합니다.
        appInfo.engineVersion = VK_MAKE_VERSION(0, 2, 2);       // 엔진 버전을 지정합니다.

         // VkInstanceCreateInfo 구조체는 Vulkan 인스턴스를 생성하기 위한 정보를 제공합니다.
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // 구조체 타입을 지정합니다.
        createInfo.pApplicationInfo = &appInfo;                    // VkApplicationInfo 구조체를 참조합니다.

        if (portabilityAlreadyAdded) {
            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR; // 포터블 비트 설정
        }

        if (enableValidationLayers) {
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

            createInfo.enabledLayerCount = static_cast<cUint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            vkengine::debug::populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

            const char* debugExtension = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

            if (find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(),
                debugExtension) != supportedInstanceExtensions.end()) {
                requiredInstanceExtensions.push_back(debugExtension);
            }

        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        // 최종 extension 설정
        createInfo.enabledExtensionCount = static_cast<cUint32_t>(requiredInstanceExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredInstanceExtensions.data();

        _VK_CHECK_RESULT_(vkCreateInstance(&createInfo, nullptr, &VKinstance));

        Log::printToLogger("Vulkan instance created successfully");
        return true;
    }

    cBool Context::createPhysicalDevice()
    {
        // 물리 디바이스 목록을 가져옵니다.
        cUint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, nullptr);

        assert(deviceCount != 0);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, devices.data());

        // 물리 디바이스를 선택한다.
        // 먼저 물리 디바이스를 선택하기 위한 함수를 정의한다.
        // Score를 기준으로 가장 높은 점수의 물리 디바이스를 선택한다.
        std::multimap<int, VkPhysicalDevice> candidates;
        std::vector<QueueFamilyIndices2> indices(deviceCount);

        uint16_t selectQueueFamilyIndeices = 0;
        for (VkPhysicalDevice& device : devices) {
            if (helper::device::isDeviceSuitableWithoutSurface(device, indices[selectQueueFamilyIndeices]))
            {
                int score = helper::device::rateDeviceSuitability(device);
                candidates.insert(std::make_pair(score, device));
                selectQueueFamilyIndeices++;
            }
        }

        int Score = 0;
        VkPhysicalDevice pdevice = VK_NULL_HANDLE;

        int i = 0;
        for (const auto& candidate : candidates) {

            if (candidate.first > Score) {
                Score = candidate.first;
                pdevice = candidate.second;
                selectQueueFamilyIndeices = i;
            }
            i++;
        }

        if (Score == 0)
        {
            Log::printToLogger("failed to find a suitable GPU!");
            return false;
        }

        this->VKdevice = DeviceManager2(pdevice, indices[selectQueueFamilyIndeices]);
        this->VKdevice.printPhysicalDeviceProperties();

        return true;
    }

    cBool Context::createLogicalDevice(cBool useSwapchain)
    {
        cBool check = true;
        
        if (!this->VKdevice.createLogicalDevice(useSwapchain))
        {
            check = false;
        }

        return check;
    }

    cBool Context::createCommandPools()
    {
        cBool check = false;

        if (this->VKdevice.createCommandPools())
        {
            check = true;
        }

        return check;
    }

    cBool Context::createQueues()
    {
        cBool check = false;

        if (this->VKdevice.createQueues())
        {
            check = true;
        }

        return check;
    }

    cBool Context::createPipelineCache()
    {
        cBool check = false;

        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        _VK_CHECK_RESULT_(vkCreatePipelineCache(this->VKdevice.logicaldevice, &pipelineCacheCreateInfo, nullptr, &this->VKpipelineCache));

        return check;
    }

    cBool Context::createDepthStencilFormat()
    {
        cBool check = true;

        this->VKdepthStencil.depthFormat = helper::device::findDepthFormat(this->VKdevice.physicalDevice);
        
        if (this->VKdepthStencil.depthFormat == VK_FORMAT_UNDEFINED) {
            check = false;
        }

        return check;
    }

    cBool Context::createDebugMessenger() {

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        debug::populateDebugMessengerCreateInfo(createInfo);

        _VK_CHECK_RESULT_(debug::CreateDebugUtilsMessengerEXT(this->VKinstance, &createInfo, nullptr, &this->VKdebugUtilsMessenger));

        return true;
    }

    cBool Context::checkValidationLayerSupport(const cChar* str)
    {
        cUint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        cBool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(str, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }

        return true;
    }

    CommandBufferHandle Context::createGraphicsCommandBufferHandle(VkCommandBufferLevel level, cBool begin)
    {
        return CommandBufferHandle(this->VKdevice.logicaldevice, this->VKdevice.graphicsCommandPool, this->VKdevice.graphicsVKQueue, level, begin);
    }
    
    std::vector<CommandBufferHandle> Context::createGraphicsCommandBufferHandles(cUint32_t count)
    {
        const VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        std::vector<VkCommandBuffer> commandBuffers(count);

        VkCommandBufferAllocateInfo cmdBufAllocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cmdBufAllocInfo.commandPool = this->VKdevice.graphicsCommandPool;
        cmdBufAllocInfo.level = level;
        cmdBufAllocInfo.commandBufferCount = count;

        _VK_CHECK_RESULT_(
            vkAllocateCommandBuffers(
                this->VKdevice.logicaldevice,
                &cmdBufAllocInfo, commandBuffers.data()));

        std::vector<CommandBufferHandle> commandBufferHandles;

        for (cUint32_t i = 0; i < count; i++) {
            commandBufferHandles.emplace_back(this->VKdevice.logicaldevice, commandBuffers[i], this->VKdevice.graphicsCommandPool, this->VKdevice.graphicsVKQueue);
        }

        return commandBufferHandles;
    }

    CommandBufferHandle Context::createTransferCommandBufferHandle(VkCommandBufferLevel level, cBool begin)
    {
        return CommandBufferHandle(this->VKdevice.logicaldevice, this->VKdevice.transferCommandPool, this->VKdevice.transferVKQueue, level, begin);
    }

    std::vector<CommandBufferHandle> Context::createTransferCommandBufferHandles(cUint32_t count)
    {
        const VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        std::vector<VkCommandBuffer> commandBuffers(count);

        VkCommandBufferAllocateInfo cmdBufAllocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cmdBufAllocInfo.commandPool = this->VKdevice.graphicsCommandPool;
        cmdBufAllocInfo.level = level;
        cmdBufAllocInfo.commandBufferCount = count;

        _VK_CHECK_RESULT_(
            vkAllocateCommandBuffers(
                this->VKdevice.logicaldevice,
                &cmdBufAllocInfo, commandBuffers.data()));

        std::vector<CommandBufferHandle> commandBufferHandles;

        for (cUint32_t i = 0; i < count; i++) {
            commandBufferHandles.emplace_back(this->VKdevice.logicaldevice, commandBuffers[i], this->VKdevice.transferCommandPool, this->VKdevice.transferVKQueue);
        }

        return commandBufferHandles;
    }

    cUint32_t Context::getMemoryTypeIndex(cUint32_t typeBits, VkMemoryPropertyFlags properties)
    {
        for (cUint32_t i = 0; i < this->VKdevice.memoryProperties.memoryTypeCount; i++) {
            if ((typeBits & 1) == 1) {
                if ((this->VKdevice.memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            typeBits >>= 1;
        }

        Log::exitToLogger("Could not find a suitable memory type.");
        return cUint32_t(-1);
    }

    void Context::waitGraphicsQueueIdle()
    {
        vkQueueWaitIdle(this->VKdevice.graphicsVKQueue);
    }
    
    void Context::waitIdle()
    {
        vkDeviceWaitIdle(this->VKdevice.logicaldevice);
    }
}