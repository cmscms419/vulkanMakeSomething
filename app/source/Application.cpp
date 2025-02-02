
#include "Application.h"
#include "DebugFunction.h"

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
        this->VKpipelineLayout = VK_NULL_HANDLE;
        this->VKrenderPass = VK_NULL_HANDLE;
        this->VKgraphicsPipeline = VK_NULL_HANDLE;
        this->VKswapChainFramebuffers.clear();
        this->VKcommandPool = VK_NULL_HANDLE;
        this->VKcommandBuffer = VK_NULL_HANDLE;
        this->VkimageavailableSemaphore = VK_NULL_HANDLE;
        this->VkrenderFinishedSemaphore = VK_NULL_HANDLE;
        this->VkinFlightFences = VK_NULL_HANDLE;
        this->state = true;
    }

    Application::~Application()
    {
    }

    void Application::init() {
        initWindow(); //    GLFW ������ ����
        initVulkan();
    }

    bool Application::run() {
        while (!glfwWindowShouldClose(VKwindow)) {
            glfwPollEvents(); // GLFW �̺�Ʈ ť ó��
            update(); // ������Ʈ
#ifdef DEBUG_
            //printf("update\n");
#endif // DEBUG_

        }

        state = false;

        return state;
    }

    void Application::update() {
        // ������� ������Ʈ
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

        vkDestroySemaphore(this->VKdevice, this->VkrenderFinishedSemaphore, nullptr);
        vkDestroySemaphore(this->VKdevice, this->VkimageavailableSemaphore, nullptr);
        vkDestroyFence(this->VKdevice, this->VkinFlightFences, nullptr);
        vkDestroyCommandPool(this->VKdevice, this->VKcommandPool, nullptr);
        for (auto framebuffer : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice, framebuffer, nullptr);
        }

        vkDestroyPipeline(this->VKdevice, this->VKgraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(this->VKdevice, this->VKpipelineLayout, nullptr);
        vkDestroyRenderPass(this->VKdevice, this->VKrenderPass, nullptr);
        
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
        glfwSetKeyCallback(this->VKwindow, vkutil::key_callback);  // Ű �Է� �ݹ� ����
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
        this->createRenderPass();
        this->createGraphicsPipeline();
        this->createFramebuffers();
        this->createCommandPool();  
        this->createCommandBuffers();
        this->createSyncObjects();
    }

    void Application::drawFrame()
    {
        // �������� �����ϱ� ���� �������� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        vkWaitForFences(this->VKdevice, 1, &this->VkinFlightFences, VK_TRUE, UINT64_MAX);

        // �̹����� �������� ���� ���� ü�ο��� �̹��� �ε����� �����ɴϴ�.
        uint32_t imageIndex;
        vkAcquireNextImageKHR(this->VKdevice, this->VKswapChain, UINT64_MAX, this->VkimageavailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        // �������� �����ϱ� ���� �̹����� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        vkResetCommandBuffer(this->VKcommandBuffer, 0);
        this->recordCommandBuffer(this->VKcommandBuffer, imageIndex);

        // �������� �����ϱ� ���� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // �������� �����ϱ� ���� ������� �����մϴ�.
        VkSemaphore waitSemaphores[] = { this->VkimageavailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        // �������� �����ϱ� ���� Ŀ�ǵ� ���۸� �����մϴ�.
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &this->VKcommandBuffer;

        // �������� �����ϱ� ���� ������� �����մϴ�.
        VkSemaphore signalSemaphores[] = { this->VkrenderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(this->VKdevice, 1, &this->VkinFlightFences);

        if (vkQueueSubmit(this->graphicsVKQueue, 1 ,&submitInfo, this->VkinFlightFences) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        
        // �������� �����ϱ� ���� ������� �����մϴ�.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { this->VKswapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pResults = nullptr;

        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(this->presentVKQueue, &presentInfo);

    }

    bool Application::mainLoop()
    {
        while (!glfwWindowShouldClose(this->VKwindow)) {
            glfwPollEvents();
            drawFrame();
#ifdef DEBUG_
            //printf("update\n");
#endif // DEBUG_

        }

        vkDeviceWaitIdle(this->VKdevice);
        state = false;

        return state;
    }

    void Application::createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // VkApplicationInfo ����ü�� ���ø����̼ǿ� ���� ������ Vulkan���� �����ϱ� ���� ���˴ϴ�.
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;     // ����ü Ÿ���� �����մϴ�.
        appInfo.pApplicationName = "Vulkan Test";               // ���ø����̼� �̸��� �����մϴ�.
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);  // ���ø����̼� ������ �����մϴ�.
        appInfo.pEngineName = "cms491 Engine";                  // ���� �̸��� �����մϴ�.
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // ���� ������ �����մϴ�.
        appInfo.apiVersion = VK_API_VERSION_1_0;                // ����� Vulkan API ������ �����մϴ�.

        // VkInstanceCreateInfo ����ü�� Vulkan �ν��Ͻ��� �����ϱ� ���� ������ �����մϴ�.
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // ����ü Ÿ���� �����մϴ�.
        createInfo.pApplicationInfo = &appInfo;                    // VkApplicationInfo ����ü�� �����մϴ�.

        uint32_t glfwExtensionCount = 0;
        std::vector<const char*> extensions = getRequiredExtensions();           // Ȯ�� ����� ������ ������ �����մϴ�.
        glfwExtensionCount = static_cast<uint32_t>(extensions.size());           // Ȯ�� ������ �����մϴ�.
        createInfo.enabledExtensionCount = glfwExtensionCount;                   // Ȱ��ȭ�� Ȯ�� ������ �����մϴ�.
        createInfo.ppEnabledExtensionNames = extensions.data();                  // Ȱ��ȭ�� Ȯ�� ����� �����մϴ�.

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

        // Vulkan �ν��Ͻ��� �����մϴ�.
        if (vkCreateInstance(&createInfo, nullptr, &VKinstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!"); // �ν��Ͻ� ���� ���� �� ���ܸ� �߻���ŵ�ϴ�.
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

        // 1. ���� ��ġ���� ť �йи� �ε����� ã���ϴ�.
        // 2. ť ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        // 3. ���� ��ġ ��� ����ü�� �ʱ�ȭ�մϴ�.
        // 4. �� ��ġ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        // 5. �� ��ġ�� �����մϴ�.
        
        // ť ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { 
            this->VKqueueFamilyIndices.graphicsFamily,
            this->VKqueueFamilyIndices.presentFamily
        };

        float queuePriority = 1.0f;                        // ť�� �켱������ �����մϴ�.
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;                          // ����ü Ÿ���� �����մϴ�.
            queueCreateInfo.queueFamilyIndex = this->VKqueueFamilyIndices.getGraphicsQueueFamilyIndex(); // �׷��Ƚ� ť �йи� �ε����� �����մϴ�.
            queueCreateInfo.queueCount = 1;                                                              // ť�� ������ �����մϴ�.
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // ���� ��ġ ��� ����ü�� �ʱ�ȭ�մϴ�.
        VkPhysicalDeviceFeatures deviceFeatures{};

        // �� ��ġ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;                          // ����ü Ÿ���� �����մϴ�.
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // ť ���� ������ ������ �����մϴ�.
        createInfo.pQueueCreateInfos = queueCreateInfos.data();                           // ť ���� ���� �����͸� �����մϴ�.
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());// Ȱ��ȭ�� Ȯ�� ������ 0���� �����մϴ�.
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();                     // Ȱ��ȭ�� Ȯ�� ����� �����մϴ�.
        createInfo.pEnabledFeatures = &deviceFeatures;                                    // ���� ��ġ ��� �����͸� �����մϴ�.

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // �� ��ġ�� �����մϴ�.
        if (vkCreateDevice(this->VKphysicalDevice, &createInfo, nullptr, &this->VKdevice) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!"); // �� ��ġ ���� ���� �� ���ܸ� �߻���ŵ�ϴ�.
        }

        // �� ����̽����� �׷��� ť �ڵ��� �����ɴϴ�.
        vkGetDeviceQueue(this->VKdevice, this->VKqueueFamilyIndices.graphicsFamily, 0, &this->graphicsVKQueue);
        // �� ����̽����� ���������̼� ť �ڵ��� �����ɴϴ�.
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

        // ���� ü�� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = this->VKsurface;

        createInfo.minImageCount = imageCount;                          // �̹��� ������ �����մϴ�.
        createInfo.imageFormat = surfaceFormat.format;                  // �̹��� ������ �����մϴ�.
        createInfo.imageColorSpace = surfaceFormat.colorSpace;          // �̹��� ���� ������ �����մϴ�.
        createInfo.imageExtent = extent;                                // �̹��� �ػ󵵸� �����մϴ�.
        createInfo.imageArrayLayers = 1;                                // �̹��� �迭 ���̾ �����մϴ�.
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;    // �̹��� ��� ����� �����մϴ�.

        // ť �йи� �ε����� �����ɴϴ�.
        uint32_t queueFamilyIndices[] = { this->VKqueueFamilyIndices.graphicsFamily, this->VKqueueFamilyIndices.presentFamily };

        // ���� ť �йи��� ���� ���� ���� ü�� �̹����� ó���ϴ� ����� ����
        if (this->VKqueueFamilyIndices.graphicsFamily != this->VKqueueFamilyIndices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // ���� ���� ��带 �����մϴ�.
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // ��Ÿ�� ���� ��带 �����մϴ�.
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // �̹��� ��ȯ�� �����մϴ�.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;            // ���� ������ �����մϴ�.
        createInfo.presentMode = presentMode;                                     // ���������̼� ��带 �����մϴ�.
        createInfo.clipped = VK_TRUE;                                             // Ŭ������ �����մϴ�. -> �ٸ� â�� �տ� �ֱ� ������ ������ �ȼ��� ������ �Ű� ���� �ʴ´ٴ� �ǹ�
        createInfo.oldSwapchain = VK_NULL_HANDLE;                                 // ���� ���� ü���� �����մϴ�. -> ���߿� ����

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

            // �̹��� �� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = this->VKswapChainImages[i];
            
            // �̹��� ���� ������ �����մϴ�.
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = this->VKswapChainImageFormat;

            // �̹����� ������Ʈ ������ �����մϴ�.
            // ���⼭�� �⺻������ R, G, B �� A ���� ��Ҹ� ����մϴ�.
            // VK_COMPONENT_SWIZZLE_IDENTITY -> ���� ���� ��Ҹ� �������� �ʰ� ���� ���� �״�� �����ϴ� ������
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            
            // �̹����� ��� ������ �����մϴ�.
            // VK_IMAGE_ASPECT_COLOR_BIT -> �̹����� ���� �����͸� �����Ѵٴ� ���� ��Ÿ���ϴ�.
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

    void Application::createRenderPass()
    {
        // ���� �н� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = this->VKswapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // ���� �н� �����н��� �����մϴ�.
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // �����н��� �����մϴ�.
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        // �����н� ���Ӽ��� �����մϴ�.
        // �ܺ� �����н��� ��� �����н��� �����մϴ�.
        // �ҽ� �������� ����ũ�� ��� �������� ����ũ�� �����մϴ�.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // �ܺ� �����н�
        dependency.dstSubpass = 0;                   // ��� �����н�
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // �ҽ� �������� ����ũ -> ���� ÷�� ��� ��Ʈ
        dependency.srcAccessMask = 0;                                            // �ҽ� �׼��� ����ũ -> 0
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // ��� �������� ����ũ -> ���� ÷�� ��� ��Ʈ
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // ��� �׼��� ����ũ -> ���� ÷�� ���� ��Ʈ

        // ���� �н� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        // ���� �н� ���� ���� ����ü�� ÷�� ���� �� �����н��� �����մϴ�.
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(this->VKdevice, &renderPassInfo, nullptr, &this->VKrenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }


    }

    void Application::createGraphicsPipeline()
    {
        auto vertShaderCode = readFile("shader/vert.spv");
        auto fragShaderCode = readFile("shader/frag.spv");
        
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



        // �׷��� ���������� ���̾ƿ��� �����մϴ�.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

        // �Է� �����͸� � ���·� ������ ������ �����մϴ�.
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // �ﰢ�� ����Ʈ�� ����
        inputAssembly.primitiveRestartEnable = VK_FALSE; // ������Ƽ�� ����� ��Ȱ��ȭ

        VkViewport viewpport{};
        viewpport.x = 0.0f;
        viewpport.y = 0.0f;
        viewpport.width = (float)this->VKswapChainExtent.width;
        viewpport.height = (float)this->VKswapChainExtent.height;
        viewpport.minDepth = 0.0f;
        viewpport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = this->VKswapChainExtent;

        // ����Ʈ ����
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewpport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // ������ȭ ����
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;                 // ���� Ŭ���� ��Ȱ��ȭ
        rasterizer.rasterizerDiscardEnable = VK_FALSE;          // ������ȭ ���� ��Ȱ��ȭ
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;          // �ٰ��� ��带 ä���� ����
        rasterizer.lineWidth = 1.0f;                            // ���� �ʺ� 1.0f�� ����
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;            // �ĸ� ���� ����
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;         // ���� ���� �ð� �������� ����
        
        rasterizer.depthBiasEnable = VK_FALSE;                  // ���� ���̾ ��Ȱ��ȭ
        rasterizer.depthBiasConstantFactor = 0.0f;              // ���� ���̾ ��� ��Ҹ� 0.0f�� ����
        rasterizer.depthBiasClamp = 0.0f;                       // ���� ���̾ Ŭ������ 0.0f�� ����
        rasterizer.depthBiasSlopeFactor = 0.0f;                 // ���� ���̾ ������ ��Ҹ� 0.0f�� ����

        // ���� ���ø� ����
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //
        multisampling.sampleShadingEnable = VK_FALSE;               // ���ø� ���̵� ��Ȱ��ȭ
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // ������ȭ ���� ���� 1��Ʈ�� ����
        multisampling.minSampleShading = 1.0f;                      // �ּ� ���ø��� 1.0f�� ���� -> option
        multisampling.pSampleMask = nullptr;                        // ���� ����ũ�� nullptr�� ���� -> option
        multisampling.alphaToCoverageEnable = VK_FALSE;             // ���� Ŀ������ ��Ȱ��ȭ -> option
        multisampling.alphaToOneEnable = VK_FALSE;                  // ���� �� ��Ȱ��ȭ -> option

        // �÷� ���� ����
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                              VK_COLOR_COMPONENT_G_BIT | 
                                              VK_COLOR_COMPONENT_B_BIT | 
                                              VK_COLOR_COMPONENT_A_BIT;     // �÷� ���� ����ũ�� ����
        colorBlendAttachment.blendEnable = VK_FALSE;                        // ������ ��Ȱ��ȭ
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     // �ҽ� �÷� ���� ���͸� ����
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;    // ��� �÷� ���� ���͸� ����
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;                // �÷� ���� ������ ����
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;     // �ҽ� ���� ���� ���͸� ����
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;    // ��� ���� ���� ���͸� ����
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;                // ���� ���� ������ ����
        /*
        if (blendEnable) {
            finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
            finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
        } else {
            finalColor = newColor;
        }
        
        finalColor = finalColor & colorWriteMask;
        */

        // �÷� ���� ���� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; // ����ü Ÿ���� ����
        colorBlending.logicOpEnable = VK_FALSE;                                       // �� ������ ��Ȱ��ȭ
        colorBlending.logicOp = VK_LOGIC_OP_COPY;                                     // �� ������ ����
        colorBlending.attachmentCount = 1;                                            // �÷� ���� ÷�� ������ ����
        colorBlending.pAttachments = &colorBlendAttachment;                           // �÷� ���� ÷�� �����͸� ����
        colorBlending.blendConstants[0] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[1] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[2] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[3] = 0.0f;                                       // ���� ����� ����

        // ���� ��� ���¸� �����մϴ�.
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // �׷��� ���������� ���̾ƿ��� �����մϴ�.
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // ����ü Ÿ���� ����
        pipelineLayoutInfo.setLayoutCount = 0;                                     // ���̾ƿ� ������ ����
        pipelineLayoutInfo.pSetLayouts = nullptr;                                  // ���̾ƿ� �����͸� ����
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // Ǫ�� ��� ���� ������ ����
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // Ǫ�� ��� ���� �����͸� ����

        if (vkCreatePipelineLayout(this->VKdevice, &pipelineLayoutInfo, nullptr, &this->VKpipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }


        // �׷��� ���������� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState; // Optional
        pipelineInfo.layout = this->VKpipelineLayout;
        pipelineInfo.renderPass = this->VKrenderPass; 
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(this->VKdevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->VKgraphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(this->VKdevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice, baseFragShaderModule, nullptr);

    }

    void Application::createFramebuffers()
    {
        this->VKswapChainFramebuffers.resize(this->VKswapChainImageViews.size());

        for (size_t i = 0; i < this->VKswapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                this->VKswapChainImageViews[i]
            };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = this->VKrenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = this->VKswapChainExtent.width;
            framebufferInfo.height = this->VKswapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(this->VKdevice, &framebufferInfo, nullptr, &this->VKswapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }

        }
    }

    void Application::createCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = this->VKqueueFamilyIndices;
        VkCommandPoolCreateInfo poolInfo{};
        
        // Ŀ�ǵ� Ǯ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;        // ����ü Ÿ���� ����
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;   // Ŀ�ǵ� ���۸� �缳���ϴ� �÷��׸� ����
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily; // ť �йи� �ε����� ����
        if (vkCreateCommandPool(this->VKdevice, &poolInfo, nullptr, &this->VKcommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void Application::createCommandBuffers()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = this->VKcommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1; // Ŀ�ǵ� ���� ������ ����

        if (vkAllocateCommandBuffers(this->VKdevice, &allocInfo, &this->VKcommandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

    }

    void Application::createSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(this->VKdevice, &semaphoreInfo, nullptr, &this->VkimageavailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(this->VKdevice, &semaphoreInfo, nullptr, &this->VkrenderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(this->VKdevice, &fenceInfo, nullptr, &this->VkinFlightFences) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }


    }

    QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices; // ť �йи��� ������ ������ ������ �ʱ�ȭ
        uint32_t queueFamilyCount = 0;

        // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ������ (ù ��° ȣ���� ������ ������)
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        // ť �йи� �Ӽ��� ������ ���͸� �ʱ�ȭ
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ������ (�� ��° ȣ���� ���� �Ӽ��� ������)
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        // ��� ť �йи��� ��ȸ
        int i = 0;
        getPyhsicalDeviceProperties(device);
        for (const auto& queueFamily : queueFamilies) {
            // ���� ť �йи��� �׷��Ƚ� ť�� �����ϴ��� Ȯ��

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
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&            // 32��Ʈ BGR ���� ������ �����ϴ��� Ȯ��
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR // SRGB ���� ������ �����ϴ��� Ȯ��
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

    void Application::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        // Ŀ�ǵ� ���� ����� �����մϴ�.
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // ���� �н� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = this->VKrenderPass;
        renderPassInfo.framebuffer = this->VKswapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = this->VKswapChainExtent;
        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // ���� �н��� �����մϴ�.
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // �׷��� ������������ ���ε��մϴ�.
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(this->VKswapChainExtent.width);
        viewport.height = static_cast<float>(this->VKswapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = this->VKswapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // ���� �н��� �����մϴ�.
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);

        // Ŀ�ǵ� ���� ����� �����մϴ�.
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
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
