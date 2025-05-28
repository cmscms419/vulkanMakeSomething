#include "VKengine.h"

using vkengine::input::key_callback;
using vkengine::VKDevice_;
using vkengine::VKSwapChain;

using namespace vkengine::debug;

namespace vkengine
{
    constexpr bool bUseValidationLayers = false;
    VulkanEngine* loadedEngine = nullptr;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    VulkanEngine::VulkanEngine(std::string root_path) {
        this->RootPath = root_path;
        this->VKwindow = nullptr;
        this->VKinstance = {};
        
        this->VKdebugUtilsMessenger = VK_NULL_HANDLE;
        
        this->VKrenderPass = std::make_unique<VkRenderPass>();
        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            this->VKframeData[i] = FrameData();
        }
    }

    VulkanEngine::~VulkanEngine() {}

    FrameData& VulkanEngine::getCurrnetFrameData()
    {
        // TODO: ���⿡ return ���� �����մϴ�.
        int index = (currentFrame) % MAX_FRAMES_IN_FLIGHT;
        return this->VKframeData[index];
    }

    VulkanEngine& VulkanEngine::Get() { return *loadedEngine; }

    void VulkanEngine::init()
    {
        assert(loadedEngine == nullptr);
        loadedEngine = this;

        this->initWindow();
        this->initVulkan();

        this->_isInitialized = true;
    }

    void VulkanEngine::cleanup()
    {
        if (this->_isInitialized)
        {
            this->VKdepthStencill.cleanup(this->VKdevice->logicaldevice);

            for (auto framebuffers : this->VKswapChainFramebuffers)
            {
                vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffers, nullptr);
            }

            this->VKswapChain->cleanupSwapChain();

            vkDestroyRenderPass(this->VKdevice->logicaldevice, *this->VKrenderPass, nullptr);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroySemaphore(this->VKdevice->logicaldevice, this->VKframeData[i].VkimageavailableSemaphore, nullptr);
                vkDestroySemaphore(this->VKdevice->logicaldevice, this->VKframeData[i].VkrenderFinishedSemaphore, nullptr);
                vkDestroyFence(this->VKdevice->logicaldevice, this->VKframeData[i].VkinFlightFences, nullptr);
            }

            vkDestroyPipelineCache(this->VKdevice->logicaldevice, this->VKpipelineCache, nullptr);

            this->VKdevice->cleanup();

            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(this->VKinstance, this->VKdebugUtilsMessenger, nullptr);
            }

            vkDestroySurfaceKHR(this->VKinstance, this->VKsurface, nullptr);
            vkDestroyInstance(this->VKinstance, nullptr);

            glfwDestroyWindow(this->VKwindow);
            glfwTerminate();
        }

        loadedEngine = nullptr;
    }

    bool VulkanEngine::mainLoop()
    {
        while (!glfwWindowShouldClose(this->VKwindow)) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(this->VKdevice->logicaldevice);
        state = false;

        return state;
    }

    void VulkanEngine::initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        int WIDTH_ = WIDTH;
        int HEIGHT_ = HEIGHT;

        this->VKwindow = glfwCreateWindow(WIDTH_, HEIGHT_, "vulkan game engine", nullptr, nullptr);

        glfwSetWindowUserPointer(this->VKwindow, this);
        glfwSetFramebufferSizeCallback(this->VKwindow, framebufferResizeCallback);
        glfwSetKeyCallback(this->VKwindow, vkengine::input::key_callback);  // Ű �Է� �ݹ� ����
        glfwSetCursorPosCallback(this->VKwindow, vkengine::input::cursorPositionCallback); // ���콺 �Է� �ݹ� ����
        glfwSetMouseButtonCallback(this->VKwindow, vkengine::input::mouseButtonCallback); // ���콺 ��ư �ݹ� ����
        glfwSetScrollCallback(this->VKwindow, vkengine::input::scroll_callback); // ��ũ�� �ݹ� ����
    }

    bool VulkanEngine::prepare()
    {
        VulkanEngine::init_command_pool();

        VulkanEngine::init_swapchain();
        
        VulkanEngine::createCommandBuffer();
        
        VulkanEngine::createDepthStencilResources();
        
        VulkanEngine::createRenderPass();
        
        VulkanEngine::createFramebuffers();
        
        VulkanEngine::init_sync_structures();
        
        VulkanEngine::createPipelineCache();

        return true;
    }

    void VulkanEngine::prepareFame(uint32_t* imageIndex)
    {
        VkResult result = this->VKswapChain->acquireNextImage(this->getCurrnetFrameData().VkimageavailableSemaphore, *imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            this->recreateSwapChain(); // ���� ü���� �ٽ� �����մϴ�.
            return;
        }
        else {
            _VK_CHECK_RESULT_(result);
        }
    }

    void VulkanEngine::presentFrame(uint32_t* imageIndex)
    {

        VkPresentInfoKHR presentInfo{};

        // VkPresentInfoKHR ����ü�� ���� ü�� �̹����� ���������̼��� ���� ������ �����մϴ�.
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; // ����ü Ÿ���� �����մϴ�.
        presentInfo.pNext = nullptr;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &this->VKframeData[currentFrame].VkrenderFinishedSemaphore;

        VkSwapchainKHR swapChains[] = { this->VKswapChain->getSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pResults = nullptr;

        presentInfo.pImageIndices = imageIndex;

        // ���������̼� ť�� �̹����� �����մϴ�.
        VkResult result = vkQueuePresentKHR(this->VKdevice->presentVKQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            this->framebufferResized = false;
            this->recreateSwapChain();
        }
        else
        {
            _VK_CHECK_RESULT_(result);
        }
    }

    void VulkanEngine::CreateDescriptorPool2(uint32_t maxFrames)
    {
        for (auto& iter : this->VKdescriptorPoolSize)
        {
            iter.descriptorCount = maxFrames;
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(this->VKdescriptorPoolSize.size());
        poolInfo.pPoolSizes = this->VKdescriptorPoolSize.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT_UI_VERSION);

        _VK_CHECK_RESULT_(vkCreateDescriptorPool(this->VKdevice->logicaldevice, &poolInfo, nullptr, &this->VKdescriptorPool));
    }

    void VulkanEngine::CreateDescriptorPool_ImGui()
    {
        VkDescriptorPoolSize pool_sizes[] =
        { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        _VK_CHECK_RESULT_(vkCreateDescriptorPool(this->VKdevice->logicaldevice, &pool_info, nullptr, &this->VKdescriptorPool));
    }

    bool VulkanEngine::initVulkan()
    {
        if (glfwVulkanSupported() == GLFW_FALSE) {
            return false;
        }

        this->createInstance();

        if (enableValidationLayers)
        {
            this->setupDebugCallback();
        }

        this->createSurface();
        this->createDevice();

        return true;
    }

    bool VulkanEngine::init_swapchain()
    {
        this->VKswapChain = std::make_unique<VKSwapChain>(this->VKdevice->physicalDevice, this->VKdevice->logicaldevice, this->VKsurface, &this->VKinstance);
        this->VKswapChain->createSwapChain(&this->VKdevice->queueFamilyIndices);
        this->VKswapChain->createImageViews();

        return true;
    }

    bool VulkanEngine::init_command_pool()
    {
        // command pool create and command buffer create
        QueueFamilyIndices queueFamilyIndices = this->VKdevice->queueFamilyIndices;

        // Ŀ�ǵ� Ǯ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkCommandPoolCreateInfo poolInfo = helper::commandPoolCreateInfo(queueFamilyIndices.graphicsAndComputeFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        _VK_CHECK_RESULT_(vkCreateCommandPool(this->VKdevice->logicaldevice, &poolInfo, nullptr, &this->VKdevice->commandPool));

        return true;
    }

    bool VulkanEngine::init_sync_structures()
    {
        VkFenceCreateInfo fenceInfo = helper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        for (auto& frameData : this->VKframeData)
        {
            _VK_CHECK_RESULT_(vkCreateFence(this->VKdevice->logicaldevice, &fenceInfo, nullptr, &frameData.VkinFlightFences));
        }

        return true;
    }

    void VulkanEngine::createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // VkApplicationInfo ����ü�� ���ø����̼ǿ� ���� ������ Vulkan���� �����ϱ� ���� ���˴ϴ�.
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;     // ����ü Ÿ���� �����մϴ�.
        appInfo.pApplicationName = "Vulkan egine";              // ���ø����̼� �̸��� �����մϴ�.
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);  // ���ø����̼� ������ �����մϴ�.
        appInfo.pEngineName = "vulkanEngine";                   // ���� �̸��� �����մϴ�.
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // ���� ������ �����մϴ�.
        appInfo.apiVersion = VK_API_VERSION_1_0;                // ����� Vulkan API ������ �����մϴ�.

        // VkInstanceCreateInfo ����ü�� Vulkan �ν��Ͻ��� �����ϱ� ���� ������ �����մϴ�.
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // ����ü Ÿ���� �����մϴ�.
        createInfo.pApplicationInfo = &appInfo;                    // VkApplicationInfo ����ü�� �����մϴ�.

        uint32_t glfwExtensionCount = 0;
        std::vector<const char*> extensions = this->getRequiredExtensions();     // Ȯ�� ����� ������ ������ �����մϴ�.
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
        _VK_CHECK_RESULT_(vkCreateInstance(&createInfo, nullptr, &this->VKinstance));
        _PRINT_TO_CONSOLE_("create instance\n");

    }

    void VulkanEngine::setupDebugCallback()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        _VK_CHECK_RESULT_(CreateDebugUtilsMessengerEXT(this->VKinstance, &createInfo, nullptr, &this->VKdebugUtilsMessenger));
    }

    void VulkanEngine::createSurface()
    {
#if CREATESURFACE_VKWIN32SURFACECREATEINFOKHR == 0
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
        //this->VKswapChain = new vkengine::VKSwapChain(this->VKdevice->VKphysicalDevice, this->VKdevice->VKdevice, this->VKsurface, &this->VKinstance);
    }

    void VulkanEngine::createDevice()
    {
        // ���� ����̽� ����� �����ɴϴ�.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, nullptr);

        assert(deviceCount != 0);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, devices.data());

        // ���� ����̽��� �����Ѵ�.
        // ���� ���� ����̽��� �����ϱ� ���� �Լ��� �����Ѵ�.
        // Score�� �������� ���� ���� ������ ���� ����̽��� �����Ѵ�.
        std::multimap<int, VkPhysicalDevice> candidates;
        std::vector<QueueFamilyIndices> indices(deviceCount);

        uint16_t selectQueueFamilyIndeices = 0;
        for (VkPhysicalDevice& device : devices) {
            if (helper::isDeviceSuitable(device, this->VKsurface, indices[selectQueueFamilyIndeices]))
            {
                int score = helper::rateDeviceSuitability(device);
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

        //this->VKmsaaSamples = helper::getMaxUsableSampleCount(this->VKphysicalDevice);

        if (Score == 0)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        this->VKdevice = std::make_unique<VKDevice_>(pdevice, indices[selectQueueFamilyIndeices]);

        // �Ļ��� ������ ������ ��ġ���� ���� �����Ǵ� Ȯ�� ��Ͽ� ���� Ȯ�� ����� Ȱ��ȭ�� �� �ֽ��ϴ�.
        // �ʿ��� �� �ڵ� ����

        // ���� ��ġ ��� ����ü�� �ʱ�ȭ�մϴ�.
        // TODO: ���߿� �ٽ� Ȯ��
        this->VKdevice->features.samplerAnisotropy = VK_TRUE; // ���÷��� ����Ͽ� �ؽ�ó�� �����մϴ�.
        this->VKdevice->features.sampleRateShading = VK_TRUE; // ���� ����Ʈ ���̵��� ����Ͽ� �ȼ��� �׸��ϴ�.

        // �� ����̽��� �����մϴ�.
        _VK_CHECK_RESULT_(this->VKdevice->createLogicalDevice());

        // depth format�� �����ɴϴ�.
        this->VKdepthStencill.depthFormat = helper::findDepthFormat(this->VKdevice->physicalDevice);
    }

    void VulkanEngine::createDepthStencilResources()
    {
        // ���� �̹��� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        this->VKdevice->createimageview(
            this->VKswapChain->getSwapChainExtent().width,
            this->VKswapChain->getSwapChainExtent().height,
            1,
            this->VKmsaaSamples,
            this->VKdepthStencill.depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->VKdepthStencill.depthImage,
            this->VKdepthStencill.depthImageMemory
        );

        // ���� �̹��� �並 �����մϴ�.
        this->VKdepthStencill.depthImageView = helper::createImageView(
            this->VKdevice->logicaldevice,
            this->VKdepthStencill.depthImage,
            this->VKdepthStencill.depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            1
        );

        // ���� �̹��� ���̾ƿ��� �����մϴ�.
        // ���߿� �ٽ� Ȯ��
        //helper::transitionImageLayout(
        //    this->VKdevice->VKdevice,
        //    this->VKdevice->VKcommandPool,
        //    this->VKdevice->graphicsVKQueue,
        //    this->VKdepthStencill.depthImage,
        //    this->VKdepthStencill.depthFormat,
        //    VK_IMAGE_LAYOUT_UNDEFINED,
        //    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        //    1
        //);
    }

    void VulkanEngine::createRenderPass()
    {
        // ���� �н� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = this->VKswapChain->getSwapChainImageFormat();
        colorAttachment.samples = this->VKmsaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Render Pass ���� �� Ŭ����
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Render Pass ���� �� ����
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // ������� ����
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // ������� ����
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // ���� ���̾ƿ��� �߿����� ����.
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // ���������̼ǿ� ���

        //// ���� ÷�� ������ �����մϴ�.
        //VkAttachmentDescription colorAttachmentResolve{};
        //colorAttachmentResolve.format = this->VKswapChain->getSwapChainImageFormat();;
        //colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        //colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        //colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // ���� ÷�� ������ �����մϴ�.
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = this->VKdepthStencill.depthFormat;
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
        //VkAttachmentReference colorAttachmentResolveRef{};
        //colorAttachmentResolveRef.attachment = 2;
        //colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // ���� ÷�� ���� ������ �����մϴ�.
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // �����н��� �����մϴ�.
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = nullptr;
        subpass.pResolveAttachments = nullptr;

        // �����н� ���Ӽ��� �����մϴ�.
        // �ܺ� �����н��� ��� �����н��� �����մϴ�.
        // �ҽ� �������� ����ũ�� ��� �������� ����ũ�� �����մϴ�.
        //�� �ڵ�� �ܺ�(�����н� ������)���� �÷�������/���ٽ� �ܰ谡 ���� ������ ��ٸ� �� 
        // ���� �����н��� �ش� ���ҽ��� �����ϰ� ������ ����ȭ�� �����մϴ�.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // �ܺ� �����н�
        dependency.dstSubpass = 0;                   // ��� �����н�
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // �ҽ� �������� ����ũ -> ���� ÷�� ��� ��Ʈ
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // ��� �������� ����ũ -> ���� ÷�� ��� ��Ʈ
        dependency.srcAccessMask = 0;                                                                                           // �ҽ� �׼��� ����ũ -> 0
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;         // ��� �׼��� ����ũ -> ���� ÷�� ���� ��Ʈ
        dependency.dependencyFlags = 0;                                                                                         // ���Ӽ� �÷��� -> 0

        // ÷�� ������ �����մϴ�.
        std::array<VkAttachmentDescription, 2> dependencies = { colorAttachment, depthAttachment };

        // ���� �н� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        // ���� �н� ���� ���� ����ü�� ÷�� ���� �� �����н��� �����մϴ�.
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pAttachments = dependencies.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        _VK_CHECK_RESULT_(vkCreateRenderPass(this->VKdevice->logicaldevice, &renderPassInfo, nullptr, this->VKrenderPass.get()));
    }

    void VulkanEngine::createPipelineCache()
    {
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        _VK_CHECK_RESULT_(vkCreatePipelineCache(this->VKdevice->logicaldevice, &pipelineCacheCreateInfo, nullptr, &this->VKpipelineCache));
    }

    void VulkanEngine::createFramebuffers()
    {
        this->VKswapChainFramebuffers.resize(this->VKswapChain->getSwapChainImages().size());

        for (size_t i = 0; i < this->VKswapChainFramebuffers.size(); i++)
        {
            const VkImageView attachments[2] = {
              this->VKswapChain->getSwapChainImageViews()[i],
              this->VKdepthStencill.depthImageView,
              // color attachment is resolved to this image
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = nullptr;
            framebufferInfo.renderPass = *this->VKrenderPass.get();
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = this->VKswapChain->getSwapChainExtent().width;
            framebufferInfo.height = this->VKswapChain->getSwapChainExtent().height;
            framebufferInfo.layers = 1;

            _VK_CHECK_RESULT_(vkCreateFramebuffer(this->VKdevice->logicaldevice, &framebufferInfo, nullptr, &this->VKswapChainFramebuffers[i]));
        }
    }

    void VulkanEngine::recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(this->VKwindow, &width, &height);

        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(this->VKwindow, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(this->VKdevice->logicaldevice);
        
        this->VKdepthStencill.cleanup(this->VKdevice->logicaldevice);

        for (auto framebuffers : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffers, nullptr);
        }
        
        this->VKswapChain->cleanupSwapChain();

        this->VKswapChain->createSwapChain(&this->VKdevice->queueFamilyIndices);  // ���� ü���� �����մϴ�.
        this->VKswapChain->createImageViews(); // �̹��� �並 �����մϴ�.
        VulkanEngine::createDepthStencilResources(); // ���� ���ٽ� ���ҽ��� �����մϴ�.
        this->createFramebuffers(); // ���� �н��� �����մϴ�.
    }

    bool VulkanEngine::createCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(this->VKdevice->commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        
        for (auto& framedata : this->VKframeData)
        {
            _VK_CHECK_RESULT_(vkAllocateCommandBuffers(this->VKdevice->logicaldevice, &allocInfo, &framedata.mainCommandBuffer));
        }


        return true;
    }

    void VulkanEngine::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
    {

    }

    bool VulkanEngine::checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> VulkanEngine::getRequiredExtensions()
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

}


