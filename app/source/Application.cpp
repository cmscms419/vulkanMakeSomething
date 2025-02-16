#include "Application.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "include/common/stb_image.h"
#endif

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace vkutil {
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    Application::Application(std::string root_path) {
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
        this->VKcommandBuffers.clear();
        this->VkimageavailableSemaphore.clear();
        this->VkrenderFinishedSemaphore.clear();
        this->VKvertexBuffer = VK_NULL_HANDLE;
        this->VKvertexBufferMemory = VK_NULL_HANDLE;
        this->VKindexBufferMemory = VK_NULL_HANDLE;
        this->VkinFlightFences.clear();
        this->VKuniformBuffers.clear();
        this->VKuniformBuffersMemory.clear();
        this->VKuniformBuffersMapped.clear();
        this->VKdescriptorPool = VK_NULL_HANDLE;
        this->VKdescriptorSetLayout = VK_NULL_HANDLE;
        this->VKdescriptorSets.clear();
        this->VKindexBuffer = VK_NULL_HANDLE;
        this->RootPath = root_path;
        this->currentFrame = 0;
        this->state = true;
        this->framebufferResized = false;
        this->VKtextureImage = VK_NULL_HANDLE;
        this->VKtextureImageMemory = VK_NULL_HANDLE;
        
        this->camera = vkutil::object::Camera();
        this->camera.setProjection(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        this->camera.setView(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
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
#endif // DEBUG_

        this->cleanupSwapChain();

        vkDestroySampler(this->VKdevice, this->VKtextureSampler, nullptr);
        vkDestroyImageView(this->VKdevice, this->VKtextureImageView, nullptr);

        vkDestroyImage(this->VKdevice, this->VKtextureImage, nullptr);
        vkFreeMemory(this->VKdevice, this->VKtextureImageMemory, nullptr);

        vkDestroyPipeline(this->VKdevice, this->VKgraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(this->VKdevice, this->VKpipelineLayout, nullptr);
        vkDestroyRenderPass(this->VKdevice, this->VKrenderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer(this->VKdevice, this->VKuniformBuffers[i], nullptr);
            vkFreeMemory(this->VKdevice, this->VKuniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(this->VKdevice, this->VKdescriptorPool, nullptr);

        vkDestroyDescriptorSetLayout(this->VKdevice, this->VKdescriptorSetLayout, nullptr);

        vkDestroyBuffer(this->VKdevice, this->VKvertexBuffer, nullptr);
        vkFreeMemory(this->VKdevice, this->VKvertexBufferMemory, nullptr);

        vkDestroyBuffer(this->VKdevice, this->VKindexBuffer, nullptr);
        vkFreeMemory(this->VKdevice, this->VKindexBufferMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(this->VKdevice, this->VkrenderFinishedSemaphore[i], nullptr);
            vkDestroySemaphore(this->VKdevice, this->VkimageavailableSemaphore[i], nullptr);
            vkDestroyFence(this->VKdevice, this->VkinFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(this->VKdevice, this->VKcommandPool, nullptr);

        vkDestroyDevice(this->VKdevice, nullptr);


        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(this->VKinstance, this->VKdebugUtilsMessenger, nullptr);
        }

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
        glfwSetFramebufferSizeCallback(this->VKwindow, framebufferResizeCallback);
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
        this->createDescriptorSetLayout();
        this->createGraphicsPipeline();
        this->createCommandPool();
        this->createColorResources();
        this->createDepthResources();
        this->createFramebuffers();
        this->createTextureImage();
        this->createTextureImageView();
        this->createTextureSampler();
        this->loadModel();
        this->createVertexBuffer();
        this->createIndexBuffer();
        this->createUniformBuffers();
        this->createDescriptorPool();
        this->createDescriptorSets();
        this->createCommandBuffers();
        this->createSyncObjects();

    }

    void Application::drawFrame()
    {
        // �������� �����ϱ� ���� �������� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        vkWaitForFences(this->VKdevice, 1, &this->VkinFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;

        // �̹����� �������� ���� ���� ü�ο��� �̹��� �ε����� �����ɴϴ�.
        // �־��� ����ü�ο��� ���� �̹����� ȹ���ϰ�, 
        // ���������� ��������� �潺�� ����Ͽ� ����ȭ�� �����ϴ� Vulkan API�� �Լ��Դϴ�.
        VkResult result = vkAcquireNextImageKHR(this->VKdevice, this->VKswapChain, UINT64_MAX, this->VkimageavailableSemaphore[this->currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            this->recreateSwapChain(); // ���� ü���� �ٽ� �����մϴ�.
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // uniform ���۸� ������Ʈ�մϴ�.
        this->updateUniformBuffer(currentFrame);

        vkResetFences(this->VKdevice, 1, &this->VkinFlightFences[currentFrame]); // �÷��׸� �缳���մϴ�. -> �������� ������ �÷��׸� �缳���մϴ�.

        // �������� �����ϱ� ���� �̹����� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        vkResetCommandBuffer(this->VKcommandBuffers[currentFrame], 0);
        this->recordCommandBuffer(this->VKcommandBuffers[currentFrame], imageIndex);

        // �������� �����ϱ� ���� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        VkSubmitInfo submitInfo{};

        // VkSubmitInfo ����ü�� ť�� ������ ��� ���۸� �����մϴ�.
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; // ����ü Ÿ���� �����մϴ�.

        // �������� �����ϱ� ���� ������� �����մϴ�.
        VkSemaphore waitSemaphores[] = { this->VkimageavailableSemaphore[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        // �������� �����ϱ� ���� Ŀ�ǵ� ���۸� �����մϴ�.
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &this->VKcommandBuffers[currentFrame];

        // �������� �����ϱ� ���� ������� �����մϴ�.
        VkSemaphore signalSemaphores[] = { this->VkrenderFinishedSemaphore[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(this->VKdevice, 1, &this->VkinFlightFences[currentFrame]); // �÷��׸� �缳���մϴ�. -> �������� ������ �÷��׸� �缳���մϴ�.

        // �������� �����մϴ�.
        if (vkQueueSubmit(this->graphicsVKQueue, 1, &submitInfo, this->VkinFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // �������� �����ϱ� ���� ������� �����մϴ�.
        VkPresentInfoKHR presentInfo{};

        // VkPresentInfoKHR ����ü�� ���� ü�� �̹����� ���������̼��� ���� ������ �����մϴ�.
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; // ����ü Ÿ���� �����մϴ�.

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { this->VKswapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pResults = nullptr;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(this->presentVKQueue, &presentInfo); // ���������̼� ť�� �̹����� �����մϴ�.

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            this->framebufferResized = false;
            this->recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        this->currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
            if (isDeviceSuitable(device))
            {
                int score = rateDeviceSuitability(device);
                candidates.insert(std::make_pair(score, device));
            }
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

        this->VKmsaaSamples = helper::getMaxUsableSampleCount(this->VKphysicalDevice);

        if (Score == 0)
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

        float queuePriority = 1.0f;                                                                      // ť�� �켱������ �����մϴ�.
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
        deviceFeatures.samplerAnisotropy = VK_TRUE; // ���÷��� ����Ͽ� �ؽ�ó�� �����մϴ�.
        deviceFeatures.sampleRateShading = VK_TRUE; // ���� ����Ʈ ���̵��� ����Ͽ� �ȼ��� �׸��ϴ�.


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
#if CREATESURFACE_VKWIN32SURFACECREATEINFOKHR
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = glfwGetWin32Window(this->VKwindow);
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(this->VKinstance, &createInfo, nullptr, &this->VKsurface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
#else
        if (glfwCreateWindowSurface(this->VKinstance, this->VKwindow, nullptr, &this->VKsurface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
#endif
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
            VKswapChainImageViews[i] = helper::createImageView(this->VKdevice, this->VKswapChainImages[i], this->VKswapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
    }

    void Application::createRenderPass()
    {
        // ���� �н� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = this->VKswapChainImageFormat;
        colorAttachment.samples = this->VKmsaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // ���� ÷�� ������ �����մϴ�.
        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = this->VKswapChainImageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // ���� ÷�� ������ �����մϴ�.
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = helper::findDepthFormat(this->VKphysicalDevice);
        depthAttachment.samples = this->VKmsaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // ���� �н� �����н��� �����մϴ�.
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // ���� ÷�� ���� ������ �����մϴ�.
        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // ���� ÷�� ���� ������ �����մϴ�.
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // �����н��� �����մϴ�.
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        // �����н� ���Ӽ��� �����մϴ�.
        // �ܺ� �����н��� ��� �����н��� �����մϴ�.
        // �ҽ� �������� ����ũ�� ��� �������� ����ũ�� �����մϴ�.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // �ܺ� �����н�
        dependency.dstSubpass = 0;                   // ��� �����н�
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // �ҽ� �������� ����ũ -> ���� ÷�� ��� ��Ʈ
        dependency.srcAccessMask = 0;                                                                                           // �ҽ� �׼��� ����ũ -> 0
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // ��� �������� ����ũ -> ���� ÷�� ��� ��Ʈ
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;         // ��� �׼��� ����ũ -> ���� ÷�� ���� ��Ʈ

        // ÷�� ������ �����մϴ�.
        std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
        
        // ���� �н� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        // ���� �н� ���� ���� ����ü�� ÷�� ���� �� �����н��� �����մϴ�.
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
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
        auto vertShaderCode = helper::readFile(this->RootPath + "/../../../../shader/vert.spv");
        auto fragShaderCode = helper::readFile(this->RootPath + "/../../../../shader/frag.spv");

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

        // vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        // �׷��� ���������� ���̾ƿ��� �����մϴ�.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // ���� ���� �ݽð� �������� ����
        rasterizer.depthBiasEnable = VK_FALSE;                  // ���� ���̾ ��Ȱ��ȭ
        //rasterizer.depthBiasConstantFactor = 0.0f;              // ���� ���̾ ��� ��Ҹ� 0.0f�� ����
        //rasterizer.depthBiasClamp = 0.0f;                       // ���� ���̾ Ŭ������ 0.0f�� ����
        //rasterizer.depthBiasSlopeFactor = 0.0f;                 // ���� ���̾ ������ ��Ҹ� 0.0f�� ����

        // ���� ���ø� ����
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //
        multisampling.sampleShadingEnable = VK_TRUE;               // ���ø� ���̵� ��Ȱ��ȭ
        multisampling.rasterizationSamples = this->VKmsaaSamples; // ������ȭ ���� ���� 1��Ʈ�� ����
        multisampling.minSampleShading = 0.2f;                      // �ּ� ���ø��� 0.2f�� ����
        multisampling.pSampleMask = nullptr;                        // ���� ����ũ�� nullptr�� ���� -> option
        multisampling.alphaToCoverageEnable = VK_FALSE;             // ���� Ŀ������ ��Ȱ��ȭ -> option
        multisampling.alphaToOneEnable = VK_FALSE;                  // ���� �� ��Ȱ��ȭ -> option

        // ���� ���ٽ� �׽�Ʈ ����
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

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
        pipelineLayoutInfo.setLayoutCount = 1;                                    // ���̾ƿ� ������ ����
        pipelineLayoutInfo.pSetLayouts = &this->VKdescriptorSetLayout;            // ���̾ƿ� �����͸� ����
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
        pipelineInfo.pDepthStencilState = &depthStencil; // Optional
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
            std::array<VkImageView, 3> attachments = {
                this->VKcolorImageView,
                this->VKdepthImageView,
                this->VKswapChainImageViews[i]
            };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = this->VKrenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
            framebufferInfo.pAttachments = attachments.data();;
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
        this->VKcommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = this->VKcommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(this->VKcommandBuffers.size()); // Ŀ�ǵ� ���� ������ ����

        if (vkAllocateCommandBuffers(this->VKdevice, &allocInfo, this->VKcommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

    }

    void Application::createSyncObjects()
    {
        this->VkimageavailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
        this->VkrenderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
        this->VkinFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(this->VKdevice, &semaphoreInfo, nullptr, &this->VkimageavailableSemaphore[i]) != VK_SUCCESS ||
                vkCreateSemaphore(this->VKdevice, &semaphoreInfo, nullptr, &this->VkrenderFinishedSemaphore[i]) != VK_SUCCESS ||
                vkCreateFence(this->VKdevice, &fenceInfo, nullptr, &this->VkinFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create semaphores!");
            }
        }



    }

    void Application::cleanupSwapChain()
    {
        vkDestroyImageView(this->VKdevice, this->VKcolorImageView, nullptr); // �÷� �̹��� �並 �����մϴ�.
        vkDestroyImage(this->VKdevice, this->VKcolorImage, nullptr); // �÷� �̹����� �����մϴ�.
        vkFreeMemory(this->VKdevice, this->VKcolorImageMemory, nullptr); // �÷� �̹��� �޸𸮸� �����մϴ�.

        vkDestroyImageView(this->VKdevice, this->VKdepthImageView, nullptr); // ���� �̹��� �並 �����մϴ�.
        vkDestroyImage(this->VKdevice, this->VKdepthImage, nullptr); // ���� �̹����� �����մϴ�.
        vkFreeMemory(this->VKdevice, this->VKdepthImageMemory, nullptr); // ���� �̹��� �޸𸮸� �����մϴ�.

        for (auto framebuffer : VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice, framebuffer, nullptr); // ������ ���۸� �����մϴ�.
        }

        for (auto swapChainView : this->VKswapChainImageViews) {
            vkDestroyImageView(this->VKdevice, swapChainView, nullptr); // �̹��� �並 �����մϴ�.
        }

        vkDestroySwapchainKHR(this->VKdevice, this->VKswapChain, nullptr); // �׷��� ������������ �����մϴ�.
    }

    void Application::recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(this->VKwindow, &width, &height);

        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(this->VKwindow, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(this->VKdevice);

        this->cleanupSwapChain(); // ���� ü���� �����մϴ�.

        this->createSwapChain();  // ���� ü���� �����մϴ�.
        this->createImageViews(); // �̹��� �並 �����մϴ�.
        this->createColorResources(); // �÷� ���ҽ��� �����մϴ�.
        this->createDepthResources(); // ���� ���ҽ��� �����մϴ�.
        this->createRenderPass(); // ���� �н��� �����մϴ�.
    }

    void Application::createVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(this->VKvertices[0]) * this->VKvertices.size();
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        helper::createBuffer(
            this->VKdevice,
            this->VKphysicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void* data;
        vkMapMemory(this->VKdevice, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, this->VKvertices.data(), (size_t)bufferSize);
        vkUnmapMemory(this->VKdevice, stagingBufferMemory);

        helper::createBuffer(
            this->VKdevice,
            this->VKphysicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->VKvertexBuffer,
            this->VKvertexBufferMemory);

        helper::copyBuffer(
            this->VKdevice, 
            this->VKcommandPool, 
            this->graphicsVKQueue, 
            stagingBuffer, 
            this->VKvertexBuffer, 
            bufferSize);

        // ������¡ ���ۿ��� ����̽� ���۷� �����͸� ������ �Ŀ��� �̸� �����ؾ� �մϴ�:
        vkDestroyBuffer(this->VKdevice, stagingBuffer, nullptr);
        vkFreeMemory(this->VKdevice, stagingBufferMemory, nullptr);
    }

    void Application::createIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(this->VKindices[0]) * this->VKindices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        helper::createBuffer(
            this->VKdevice,
            this->VKphysicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        
        void* data;
        vkMapMemory(this->VKdevice, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, this->VKindices.data(), (size_t)bufferSize);
        vkUnmapMemory(this->VKdevice, stagingBufferMemory);
        
        helper::createBuffer(
            this->VKdevice,
            this->VKphysicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->VKindexBuffer,
            this->VKindexBufferMemory);

        helper::copyBuffer(
            this->VKdevice,
            this->VKcommandPool,
            this->graphicsVKQueue,
            stagingBuffer,
            this->VKindexBuffer,
            bufferSize);

        vkDestroyBuffer(this->VKdevice, stagingBuffer, nullptr);
        vkFreeMemory(this->VKdevice, stagingBufferMemory, nullptr);

    }

    void Application::createDescriptorSetLayout()
    {
        // ��ũ���� ��Ʈ ���̾ƿ� ���ε��� �����մϴ�.
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        // ��ũ���� ��Ʈ ���̾ƿ� ���� ���� ����ü�� �ʱ�ȭ�մϴ�. -> ���� �ڵ� �����
        //VkDescriptorSetLayoutCreateInfo layoutInfo{};
        //layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        //layoutInfo.bindingCount = 1;
        //layoutInfo.pBindings = &uboLayoutBinding;

        // ���÷� ���̾ƿ� ���ε��� �����մϴ�.
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        // ��ũ���� ��Ʈ ���̾ƿ� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(this->VKdevice, &layoutInfo, nullptr, &this->VKdescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }


    }

    void Application::createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(object::UniformBufferObject);

        this->VKuniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        this->VKuniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        this->VKuniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            helper::createBuffer(
                this->VKdevice,
                this->VKphysicalDevice,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                this->VKuniformBuffers[i],
                this->VKuniformBuffersMemory[i]);

            vkMapMemory(this->VKdevice, this->VKuniformBuffersMemory[i], 0, bufferSize, 0, &this->VKuniformBuffersMapped[i]);
        }
    }

    void Application::createDescriptorPool()
    {
        /* ���� �ڵ� ����� */
        //// ��ũ���� Ǯ ũ�⸦ �����մϴ�.
        //VkDescriptorPoolSize poolSize{};
        //poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        //
        //// ��ũ���� Ǯ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        //VkDescriptorPoolCreateInfo poolInfo{};
        //poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        //poolInfo.poolSizeCount = 1;
        //poolInfo.pPoolSizes = &poolSize;
        //poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        //// ��ũ���� Ǯ ũ�⸦ �����մϴ�.
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        // ��ũ���� Ǯ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(this->VKdevice, &poolInfo, nullptr, &this->VKdescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }

    }

    void Application::createDescriptorSets()
    {
        // ��ũ���� ��Ʈ ���̾ƿ��� �����մϴ�.
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, this->VKdescriptorSetLayout);

        // ��ũ���� ��Ʈ �Ҵ� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkDescriptorSetAllocateInfo allocInfo{};

        // ��ũ���� ��Ʈ �Ҵ� ���� ����ü�� ��ũ���� Ǯ�� ��ũ���� ��Ʈ ������ �����մϴ�.
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = this->VKdescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        // ��ũ���� ��Ʈ�� �����մϴ�.
        this->VKdescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        // ��ũ���� ��Ʈ�� �Ҵ��մϴ�.
        if (vkAllocateDescriptorSets(this->VKdevice, &allocInfo, this->VKdescriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        
        // ��ũ���� ��Ʈ�� �����մϴ�.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            
            // ��ũ���� ���� ������ �����մϴ�.
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = this->VKuniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(object::UniformBufferObject);

            // ��ũ���� �̹��� ������ �����մϴ�.
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = this->VKtextureImageView;
            imageInfo.sampler = this->VKtextureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = this->VKdescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = this->VKdescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(this->VKdevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void Application::createTextureImage()
    {
        int texWidth, texHeight, texChannels;
        //stbi_uc* pixels = stbi_load((this->RootPath + "/../../../../source/image.jpg").c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        std::string objtex = this->RootPath + TEXTURE_PATH;

        stbi_uc* pixels = stbi_load(objtex.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        this->VKmipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        helper::createBuffer(
            this->VKdevice,
            this->VKphysicalDevice,
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);
        
        void* data;
        vkMapMemory(this->VKdevice, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(this->VKdevice, stagingBufferMemory);

        stbi_image_free(pixels);

        helper::createImage(
            this->VKdevice,
            this->VKphysicalDevice,
            texWidth,
            texHeight,
            this->VKmipLevels,
            VK_SAMPLE_COUNT_1_BIT,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->VKtextureImage,
            this->VKtextureImageMemory);
        helper::transitionImageLayout(
            this->VKdevice,
            this->VKcommandPool,
            this->graphicsVKQueue,
            this->VKtextureImage,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            this->VKmipLevels);
        helper::copyBufferToImage(
            this->VKdevice,
            this->VKcommandPool,
            this->graphicsVKQueue,
            stagingBuffer,
            this->VKtextureImage,
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight));
        // �Ӹ��� �����ϴ� ���� VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL�� ��ȯ�Ǿ����ϴ�.
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL�� ��ȯ�Ǹ� �̹����� ���̴����� ���� �� �ֽ��ϴ�.
        // �׷��� �Ʒ��� �ڵ�� ����

        vkDestroyBuffer(this->VKdevice, stagingBuffer, nullptr);
        vkFreeMemory(this->VKdevice, stagingBufferMemory, nullptr);

        helper::generateMipmaps(
            this->VKphysicalDevice,
            this->VKdevice,
            this->VKcommandPool,
            this->graphicsVKQueue,
            this->VKtextureImage,
            VK_FORMAT_R8G8B8A8_SRGB,
            texWidth,
            texHeight,
            this->VKmipLevels);
    }

    void Application::createTextureImageView()
    {
        // �̹��� �� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        this->VKtextureImageView = 
            helper::createImageView(
            this->VKdevice,
            this->VKtextureImage,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT,
            this->VKmipLevels);
    }

    void Application::createTextureSampler()
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(this->VKphysicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f; // Optional
        samplerInfo.maxLod = static_cast<float>(this->VKmipLevels);
        samplerInfo.mipLodBias = 0.0f; // Optional

        if (vkCreateSampler(this->VKdevice, &samplerInfo, nullptr, &this->VKtextureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void Application::createDepthResources()
    {
        // ���� �̹����� �����մϴ�.
        VkFormat depthFormat = helper::findDepthFormat(this->VKphysicalDevice);

        // �̹��� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        helper::createImage(
            this->VKdevice,
            this->VKphysicalDevice,
            this->VKswapChainExtent.width,
            this->VKswapChainExtent.height,
            1,
            this->VKmsaaSamples,
            depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->VKdepthImage,
            this->VKdepthImageMemory);
        
        // �̹��� �並 �����մϴ�.
        this->VKdepthImageView = helper::createImageView(
            this->VKdevice,
            this->VKdepthImage,
            depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            1);

        // ���� �̹��� ���̾ƿ��� �����մϴ�.
        helper::transitionImageLayout(
            this->VKdevice,
            this->VKcommandPool,
            this->graphicsVKQueue,
            this->VKdepthImage,
            depthFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            1);
    }

    void Application::loadModel()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string obj = this->RootPath + MODEL_PATH;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                                                                

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };
#if UNIQUE_VERTEXTYPE

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(this->VKvertices.size());
                    this->VKvertices.push_back(vertex);
                }
                this->VKindices.push_back(uniqueVertices[vertex]);
#else
                
                this->VKvertices.push_back(vertex);
                this->VKindices.push_back(this->VKindices.size());
#endif
            }
        }
    }

    void Application::createColorResources()
    {
        VkFormat colorFormat = this->VKswapChainImageFormat;

        helper::createImage(
            this->VKdevice,
            this->VKphysicalDevice,
            this->VKswapChainExtent.width,
            this->VKswapChainExtent.height,
            1,
            this->VKmsaaSamples,
            colorFormat,
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            this->VKcolorImage, 
            this->VKcolorImageMemory);

        this->VKcolorImageView = helper::createImageView(this->VKdevice, this->VKcolorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

    const QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device)
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

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    bool Application::checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {

#ifdef DEBUG_
            std::cout << '\t' << extension.extensionName << '\n';
#endif // DEBUG

            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    const  SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice device)
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

    const VkShaderModule Application::createShaderModule(const std::vector<char>& code)
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

        // ���� �н��� �����ϱ� ���� Ŭ���� �� ����
        std::array<VkClearValue, 2> clearValues{};
        //clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
        renderPassInfo.pClearValues = clearValues.data();

        // ���� �н��� �����մϴ�.
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
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

            // ���ؽ� ���۸� ���ε��մϴ�.
            VkBuffer vertexBuffers[] = { this->VKvertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            
            // �ε��� ���۸� ���ε��մϴ�.
            vkCmdBindIndexBuffer(commandBuffer, this->VKindexBuffer, 0, VK_INDEX_TYPE_UINT32);

            // ��ũ���� ��Ʈ�� ���ε��մϴ�.
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKpipelineLayout, 0, 1, &this->VKdescriptorSets[currentFrame], 0, nullptr);
            // ���� �н��� �����մϴ�.
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(this->VKindices.size()), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffer);

        // Ŀ�ǵ� ���� ����� �����մϴ�.
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void Application::updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        object::UniformBufferObject ubo{};

        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = this->camera.getViewMatrix();
        //ubo.proj = glm::perspective(glm::radians(45.0f), this->VKswapChainExtent.width / (float)this->VKswapChainExtent.height, 0.1f, 10.0f);
        ubo.proj = this->camera.getProjectionMatrix();

        memcpy(this->VKuniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
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
}
