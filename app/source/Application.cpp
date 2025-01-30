
#include "../../include/source/Application.h"
#include "../../include/source/DebugFunction.h"

namespace vkutil {

    Application::Application() {
        this->VKwindow = nullptr;
        this->VKinstance = {};
        this->VKdebugUtilsMessenger = VK_NULL_HANDLE;
        this->VKphysicalDevice = VK_NULL_HANDLE;
        this->VKqueueFamilyIndices = {};
        this->VKdevice = VK_NULL_HANDLE;
        this->graphicsVKQueue = VK_NULL_HANDLE;
        this->presentVKQueue = VK_NULL_HANDLE;
        this->VKsurface = VK_NULL_HANDLE;
        this->VKswapChain = VK_NULL_HANDLE;
        this->VKswapChainImages.clear();
        this->VKswapChainImageViews.clear();
        this->VKswapChainImageFormat = VK_FORMAT_UNDEFINED;
        this->VKswapChainExtent = { 0, 0 };
        this->state = true;
    }

    Application::~Application()
    {
    }

    void Application::init() {
        initWindow(); //    GLFW 윈도우 생성
        initVulkan();
    }

    bool Application::run() {
        while (!glfwWindowShouldClose(VKwindow)) {
            glfwPollEvents(); // GLFW 이벤트 큐 처리
            update(); // 업데이트
#ifdef DEBUG_
            //printf("update\n");
#endif // DEBUG_

        }

        state = false;

        return state;
    }

    void Application::update() {
        // 만든것을 업데이트
    }

    void Application::setup()
    {

    }

    void Application::cleanup() {

#ifdef DEBUG_

        printf("cleanup\n");
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(this->VKinstance, this->VKdebugUtilsMessenger, nullptr);
        }

#endif // DEBUG_
        for (auto imageView : this->VKswapChainImageViews) {
            vkDestroyImageView(this->VKdevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(this->VKdevice, this->VKswapChain, nullptr);
        vkDestroyDevice(this->VKdevice, nullptr);
        vkDestroySurfaceKHR(this->VKinstance, this->VKsurface, nullptr);
        vkDestroyInstance(this->VKinstance, nullptr);
        glfwDestroyWindow(VKwindow);
        glfwTerminate();
    }

    void Application::initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        int WIDTH_ = WIDTH;
        int HEIGHT_ = HEIGHT;

        this->VKwindow = glfwCreateWindow(WIDTH_, HEIGHT_, "Vulkan Test", nullptr, nullptr);

        glfwSetWindowUserPointer(this->VKwindow, this);
        glfwSetKeyCallback(this->VKwindow, vkutil::key_callback);  // 키 입력 콜백 설정
    }

    void Application::initVulkan() {

        if (glfwVulkanSupported() == GLFW_FALSE) {
            throw std::runtime_error("Vulkan is not supported");
        }

        this->createInstance();
        this->setupDebugCallback();
        this->createSurface();
        this->pickPhysicalDevice();
        this->createLogicalDevice();
        this->createSwapChain();
        this->createImageViews();
        this->createGraphicsPipeline();
    }

    void Application::mainLoop()
    {

    }

    void Application::createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // VkApplicationInfo 구조체는 애플리케이션에 대한 정보를 Vulkan에게 제공하기 위해 사용됩니다.
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;     // 구조체 타입을 지정합니다.
        appInfo.pApplicationName = "Vulkan Test";               // 애플리케이션 이름을 지정합니다.
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);  // 애플리케이션 버전을 지정합니다.
        appInfo.pEngineName = "cms491 Engine";                  // 엔진 이름을 지정합니다.
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // 엔진 버전을 지정합니다.
        appInfo.apiVersion = VK_API_VERSION_1_0;                // 사용할 Vulkan API 버전을 지정합니다.

        // VkInstanceCreateInfo 구조체는 Vulkan 인스턴스를 생성하기 위한 정보를 제공합니다.
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // 구조체 타입을 지정합니다.
        createInfo.pApplicationInfo = &appInfo;                    // VkApplicationInfo 구조체를 참조합니다.

        uint32_t glfwExtensionCount = 0;
        std::vector<const char*> extensions = getRequiredExtensions();           // 확장 목록을 저장할 변수를 선언합니다.
        glfwExtensionCount = static_cast<uint32_t>(extensions.size());           // 확장 개수를 저장합니다.
        createInfo.enabledExtensionCount = glfwExtensionCount;                   // 활성화할 확장 개수를 지정합니다.
        createInfo.ppEnabledExtensionNames = extensions.data();                  // 활성화할 확장 목록을 지정합니다.

        if (enableValidationLayers) {

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // Vulkan 인스턴스를 생성합니다.
        if (vkCreateInstance(&createInfo, nullptr, &VKinstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!"); // 인스턴스 생성 실패 시 예외를 발생시킵니다.
        }
        else
        {
            printf("create instance\n");
        }

    }

    void Application::setupDebugCallback()
    {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(this->VKinstance, &createInfo, nullptr, &this->VKdebugUtilsMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void Application::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, devices.data());

        // Use an ordered map to automatically sort candidates by increasing score
        std::multimap<int, VkPhysicalDevice> candidates;

        for (const auto& device : devices) {
            int score = rateDeviceSuitability(device);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        int Score = 0;

        for (const auto& candidate : candidates) {
            if (candidate.first > Score) {
                Score = candidate.first;
                this->VKphysicalDevice = candidate.second;
                this->VKqueueFamilyIndices = this->findQueueFamilies(candidate.second);
            }
        }
        
        if(Score == 0) 
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

#ifdef DEBUG_

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(this->VKphysicalDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(this->VKphysicalDevice, &deviceFeatures);

        printf("Select Device\n");
        printf("Select DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
        printf("Select Device Name: %s\n", deviceProperties.deviceName);

#endif // DEBUG_

    }

    void Application::createLogicalDevice() {

        // 1. 물리 장치에서 큐 패밀리 인덱스를 찾습니다.
        // 2. 큐 생성 정보 구조체를 초기화합니다.
        // 3. 물리 장치 기능 구조체를 초기화합니다.
        // 4. 논리 장치 생성 정보 구조체를 초기화합니다.
        // 5. 논리 장치를 생성합니다.
        
        // 큐 생성 정보 구조체를 초기화합니다.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { 
            this->VKqueueFamilyIndices.graphicsFamily,
            this->VKqueueFamilyIndices.presentFamily
        };

        float queuePriority = 1.0f;                        // 큐의 우선순위를 설정합니다.
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;                          // 구조체 타입을 지정합니다.
            queueCreateInfo.queueFamilyIndex = this->VKqueueFamilyIndices.getGraphicsQueueFamilyIndex(); // 그래픽스 큐 패밀리 인덱스를 설정합니다.
            queueCreateInfo.queueCount = 1;                                                              // 큐의 개수를 설정합니다.
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 물리 장치 기능 구조체를 초기화합니다.
        VkPhysicalDeviceFeatures deviceFeatures{};

        // 논리 장치 생성 정보 구조체를 초기화합니다.
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;                          // 구조체 타입을 지정합니다.
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // 큐 생성 정보의 개수를 설정합니다.
        createInfo.pQueueCreateInfos = queueCreateInfos.data();                           // 큐 생성 정보 포인터를 설정합니다.
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());// 활성화할 확장 개수를 0으로 설정합니다.
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();                     // 활성화할 확장 목록을 설정합니다.
        createInfo.pEnabledFeatures = &deviceFeatures;                                    // 물리 장치 기능 포인터를 설정합니다.

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // 논리 장치를 생성합니다.
        if (vkCreateDevice(this->VKphysicalDevice, &createInfo, nullptr, &this->VKdevice) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!"); // 논리 장치 생성 실패 시 예외를 발생시킵니다.
        }

        // 논리 디바이스에서 그래픽 큐 핸들을 가져옵니다.
        vkGetDeviceQueue(this->VKdevice, this->VKqueueFamilyIndices.graphicsFamily, 0, &this->graphicsVKQueue);
        // 논리 디바이스에서 프레젠테이션 큐 핸들을 가져옵니다.
        vkGetDeviceQueue(this->VKdevice, this->VKqueueFamilyIndices.presentFamily, 0, &this->presentVKQueue);
    }

    void Application::createSurface()
    {
        if (glfwCreateWindowSurface(this->VKinstance, this->VKwindow, nullptr, &this->VKsurface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void Application::createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(this->VKphysicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // 스왑 체인 생성 정보 구조체를 초기화합니다.
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = this->VKsurface;

        createInfo.minImageCount = imageCount;                          // 이미지 개수를 설정합니다.
        createInfo.imageFormat = surfaceFormat.format;                  // 이미지 형식을 설정합니다.
        createInfo.imageColorSpace = surfaceFormat.colorSpace;          // 이미지 색상 공간을 설정합니다.
        createInfo.imageExtent = extent;                                // 이미지 해상도를 설정합니다.
        createInfo.imageArrayLayers = 1;                                // 이미지 배열 레이어를 설정합니다.
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;    // 이미지 사용 방법을 설정합니다.

        // 큐 패밀리 인덱스를 가져옵니다.
        uint32_t queueFamilyIndices[] = { this->VKqueueFamilyIndices.graphicsFamily, this->VKqueueFamilyIndices.presentFamily };

        // 여러 큐 패밀리에 걸쳐 사용될 스왑 체인 이미지를 처리하는 방법을 지정
        if (this->VKqueueFamilyIndices.graphicsFamily != this->VKqueueFamilyIndices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // 동시 공유 모드를 설정합니다.
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // 배타적 공유 모드를 설정합니다.
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // 이미지 변환을 설정합니다.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;            // 알파 블렌딩을 설정합니다.
        createInfo.presentMode = presentMode;                                     // 프레젠테이션 모드를 설정합니다.
        createInfo.clipped = VK_TRUE;                                             // 클리핑을 설정합니다. -> 다른 창이 앞에 있기 때문에 가려진 픽셀의 색상을 신경 쓰지 않는다는 의미
        createInfo.oldSwapchain = VK_NULL_HANDLE;                                 // 이전 스왑 체인을 설정합니다. -> 나중에 설정

        if (vkCreateSwapchainKHR(this->VKdevice, &createInfo, nullptr, &this->VKswapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(this->VKdevice, this->VKswapChain, &imageCount, nullptr);
        this->VKswapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(this->VKdevice, this->VKswapChain, &imageCount, this->VKswapChainImages.data());

        this->VKswapChainImageFormat = surfaceFormat.format;
        this->VKswapChainExtent = extent;

#ifdef DEBUG_
        printf("create swap chain\n");
        printf("SwapChain imageCount: %d\n", imageCount);
        printf("SwapChain imageFormat: %d\n", surfaceFormat.format);
        printf("SwapChain imageColorSpace: %d\n", surfaceFormat.colorSpace);
        printf("SwapChain imageExtent.width: %d\n", extent.width);
        printf("SwapChain imageExtent.height: %d\n", extent.height);
        printf("SwapChain imageArrayLayers: %d\n", 1);
        printf("SwapChain imageUsage: %d\n", VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        printf("SwapChain imageSharingMode: %d\n", VK_SHARING_MODE_EXCLUSIVE);
        printf("SwapChain preTransform: %d\n", swapChainSupport.capabilities.currentTransform);
        printf("SwapChain compositeAlpha: %d\n", VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
        printf("SwapChain presentMode: %d\n", presentMode);
        printf("SwapChain clipped: %d\n", VK_TRUE);
        printf("\n");
       
#endif // DEBUG_


    }

    void Application::createImageViews()
    {
        this->VKswapChainImageViews.resize(this->VKswapChainImages.size()); // 

        for (int8_t i = 0; i < this->VKswapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};

            // 이미지 뷰 생성 정보 구조체를 초기화합니다.
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = this->VKswapChainImages[i];
            
            // 이미지 뷰의 유형을 설정합니다.
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = this->VKswapChainImageFormat;

            // 이미지의 컴포넌트 매핑을 설정합니다.
            // 여기서는 기본적으로 R, G, B 및 A 구성 요소를 사용합니다.
            // VK_COMPONENT_SWIZZLE_IDENTITY -> 색상 구성 요소를 변경하지 않고 원래 값을 그대로 유지하는 스와젤
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            
            // 이미지의 사용 목적을 설정합니다.
            // VK_IMAGE_ASPECT_COLOR_BIT -> 이미지가 색상 데이터를 포함한다는 것을 나타냅니다.
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(this->VKdevice, &createInfo, nullptr, &this->VKswapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void Application::createGraphicsPipeline()
    {
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");
        
        VkShaderModule baseVertshaderModule = createShaderModule(vertShaderCode);
        VkShaderModule baseFragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = baseVertshaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = baseFragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        vkDestroyShaderModule(this->VKdevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice, baseFragShaderModule, nullptr);

    }

    QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices; // 큐 패밀리의 개수를 저장할 변수를 초기화
        uint32_t queueFamilyCount = 0;

        // 주어진 물리 장치에서 큐 패밀리 속성을 가져옴 (첫 번째 호출은 개수만 가져옴)
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        // 큐 패밀리 속성을 저장할 벡터를 초기화
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        // 주어진 물리 장치에서 큐 패밀리 속성을 가져옴 (두 번째 호출은 실제 속성을 가져옴)
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        // 모든 큐 패밀리를 순회
        int i = 0;
        getPyhsicalDeviceProperties(device);
        for (const auto& queueFamily : queueFamilies) {
            // 현재 큐 패밀리가 그래픽스 큐를 지원하는지 확인

#ifdef DEBUG_
            printf("QueueFamily %d\n", i);
            printf("QueueFamily queueCount: %d\n", queueFamily.queueCount);
            printf("QueueFamily queueFlags: %d\n", queueFamily.queueFlags);
            printf("QueueFamily timestampValidBits: %d\n", queueFamily.timestampValidBits);
            printf("QueueFamily minImageTransferGranularity.width: %d\n", queueFamily.minImageTransferGranularity.width);
            printf("QueueFamily minImageTransferGranularity.height: %d\n", queueFamily.minImageTransferGranularity.height);
            printf("QueueFamily minImageTransferGranularity.depth: %d\n", queueFamily.minImageTransferGranularity.depth);
#endif // DEBUG_

            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) {

                printf("VK_QUEUE_GRAPHICS_BIT is supported\n");
                indices.setGraphicsFamily(i);
            }

            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
            {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->VKsurface, &presentSupport);

                if (presentSupport)
                {
                    printf("VK_QUEUE_PRESENT_BIT is supported\n");
                    indices.setPresentFamily(i);
                }

            }

            if (indices.isComplete()) {
                indices.queueFamilyProperties = queueFamily;
                printf("Queuefamily index: %d\n", i);
                printf("\n");
                break;
            }

            indices.reset();
            i++;
        }

        return indices;
    }

    bool Application::isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = this->findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;

        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool Application::checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->VKsurface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->VKsurface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->VKsurface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->VKsurface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->VKsurface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        VkSurfaceFormatKHR targetformat = {};

        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&            // 32비트 BGR 색상 구조를 지원하는지 확인
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR // SRGB 색상 공간을 지원하는지 확인
                ) 
            {
                targetformat = availableFormat;
                break;
            }
        }


        return targetformat;
    }

    VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(this->VKwindow, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }

    }

    VkShaderModule Application::createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(this->VKdevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    int rateDeviceSuitability(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        int score = 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader) {
            return 0;
        }

#ifdef DEBUG_
        printf("Device %s score: %d\n", deviceProperties.deviceName, score);
        printf("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
        printf("Device Name: %s\n", deviceProperties.deviceName);
        printf("\n");
#endif // DEBUG_


        return score;
    }

    std::vector<const char*> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void getPyhsicalDeviceProperties(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        printf("Device Name: %s\n", deviceProperties.deviceName);
        printf("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        return true;
    }
}
