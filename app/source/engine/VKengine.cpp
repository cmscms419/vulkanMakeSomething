#include "VKengine.h"
#include "VKkey.h"
#include "Debug.h"
#include "helper.h"

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
            this->VKdepthStencill.cleanup(this->VKdevice->VKdevice);

            for (auto framebuffers : this->VKswapChainFramebuffers)
            {
                vkDestroyFramebuffer(this->VKdevice->VKdevice, framebuffers, nullptr);
            }

            this->VKswapChain->cleanupSwapChain();

            vkDestroyRenderPass(this->VKdevice->VKdevice, this->VKrenderPass, nullptr);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroySemaphore(this->VKdevice->VKdevice, this->VKframeData[i].VkimageavailableSemaphore, nullptr);
                vkDestroySemaphore(this->VKdevice->VKdevice, this->VKframeData[i].VkrenderFinishedSemaphore, nullptr);
                vkDestroyFence(this->VKdevice->VKdevice, this->VKframeData[i].VkinFlightFences, nullptr);
            }

            vkDestroyPipelineCache(this->VKdevice->VKdevice, this->VKpipelineCache, nullptr);

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
#ifdef DEBUG_
            //printf("update\n");
#endif // DEBUG_

        }

        vkDeviceWaitIdle(this->VKdevice->VKdevice);
        state = false;

        return state;
    }

    void VulkanEngine::initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        int WIDTH_ = WIDTH;
        int HEIGHT_ = HEIGHT;

        this->VKwindow = glfwCreateWindow(WIDTH_, HEIGHT_, "vulkan game engine 000", nullptr, nullptr);

        glfwSetWindowUserPointer(this->VKwindow, this);
        glfwSetFramebufferSizeCallback(this->VKwindow, framebufferResizeCallback);
        glfwSetKeyCallback(this->VKwindow, vkengine::input::key_callback);  // Ű �Է� �ݹ� ����

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
            VK_CHECK_RESULT(result);
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
            VK_CHECK_RESULT(result);
        }
    }

    bool VulkanEngine::initVulkan()
    {
        if (glfwVulkanSupported() == GLFW_FALSE) {
            throw std::runtime_error("Vulkan is not supported");
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
        this->VKswapChain = new VKSwapChain(this->VKdevice->VKphysicalDevice, this->VKdevice->VKdevice, this->VKsurface, &this->VKinstance);
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

        VK_CHECK_RESULT(vkCreateCommandPool(this->VKdevice->VKdevice, &poolInfo, nullptr, &this->VKdevice->VKcommandPool));

        return true;
    }

    bool VulkanEngine::init_sync_structures()
    {
        VkFenceCreateInfo fenceInfo = helper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        for (auto& frameData : this->VKframeData)
        {
            VK_CHECK_RESULT(vkCreateFence(this->VKdevice->VKdevice, &fenceInfo, nullptr, &frameData.VkinFlightFences));
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
        if (vkCreateInstance(&createInfo, nullptr, &this->VKinstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!"); // �ν��Ͻ� ���� ���� �� ���ܸ� �߻���ŵ�ϴ�.
        }
        else
        {
            printf("create instance\n");
        }
    }

    void VulkanEngine::setupDebugCallback()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(this->VKinstance, &createInfo, nullptr, &this->VKdebugUtilsMessenger));
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

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, devices.data());

        // ���� ����̽��� �����Ѵ�.
        // ���� ���� ����̽��� �����ϱ� ���� �Լ��� �����Ѵ�.
        // Score�� �������� ���� ���� ������ ���� ����̽��� �����Ѵ�.
        std::multimap<int, VkPhysicalDevice> candidates;
        std::vector<QueueFamilyIndices> indices(deviceCount);

        uint16_t selectQueueFamilyIndeices = 0;
        for (const auto& device : devices) {
            if (helper::isDeviceSuitable(device, this->VKsurface, &indices[selectQueueFamilyIndeices]))
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

        this->VKdevice = new vkengine::VKDevice_(pdevice, indices[selectQueueFamilyIndeices]);

        // �Ļ��� ������ ������ ��ġ���� ���� �����Ǵ� Ȯ�� ��Ͽ� ���� Ȯ�� ����� Ȱ��ȭ�� �� �ֽ��ϴ�.
        // �ʿ��� �� �ڵ� ����

        // ���� ��ġ ��� ����ü�� �ʱ�ȭ�մϴ�.
        this->VKdevice->features.samplerAnisotropy = VK_TRUE; // ���÷��� ����Ͽ� �ؽ�ó�� �����մϴ�.
        this->VKdevice->features.sampleRateShading = VK_TRUE; // ���� ����Ʈ ���̵��� ����Ͽ� �ȼ��� �׸��ϴ�.

        // �� ����̽��� �����մϴ�.
        VkResult result = this->VKdevice->createLogicalDevice();

        // depth format�� �����ɴϴ�.
        this->VKdepthStencill.depthFormat = helper::findDepthFormat(this->VKdevice->VKphysicalDevice);
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
            this->VKdevice->VKdevice,
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
        dependency.srcAccessMask = 0;                                                                                           // �ҽ� �׼��� ����ũ -> 0
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // ��� �������� ����ũ -> ���� ÷�� ��� ��Ʈ
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

        if (vkCreateRenderPass(this->VKdevice->VKdevice, &renderPassInfo, nullptr, &this->VKrenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void VulkanEngine::createPipelineCache()
    {
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        VK_CHECK_RESULT(vkCreatePipelineCache(this->VKdevice->VKdevice, &pipelineCacheCreateInfo, nullptr, &this->VKpipelineCache));
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
            framebufferInfo.renderPass = this->VKrenderPass;
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = this->VKswapChain->getSwapChainExtent().width;
            framebufferInfo.height = this->VKswapChain->getSwapChainExtent().height;
            framebufferInfo.layers = 1;

            VK_CHECK_RESULT(vkCreateFramebuffer(this->VKdevice->VKdevice, &framebufferInfo, nullptr, &this->VKswapChainFramebuffers[i]));
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

        vkDeviceWaitIdle(this->VKdevice->VKdevice);

        this->VKswapChain->createSwapChain(&this->VKdevice->queueFamilyIndices);  // ���� ü���� �����մϴ�.
        this->VKswapChain->createImageViews(); // �̹��� �並 �����մϴ�.
        this->createRenderPass(); // ���� �н��� �����մϴ�.
    }

    bool VulkanEngine::createCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(this->VKdevice->VKcommandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        
        for (auto& framedata : this->VKframeData)
        {
            VK_CHECK_RESULT(vkAllocateCommandBuffers(this->VKdevice->VKdevice, &allocInfo, &framedata.mainCommandBuffer));
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


