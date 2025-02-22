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
    }

    VulkanEngine::~VulkanEngine()
    {
        this->cleanup();
    }

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
        this->init_vulkan();
        this->prepare();
    }

    void VulkanEngine::cleanup()
    {
        if (this->_isInitialized)
        {
            //this->VKswapChain->~VKSwapChain();
            
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroySemaphore(this->VKdevice->VKdevice, this->VKframeData[i].VkimageavailableSemaphore, nullptr);
                vkDestroySemaphore(this->VKdevice->VKdevice, this->VKframeData[i].VkrenderFinishedSemaphore, nullptr);
                vkDestroyFence(this->VKdevice->VKdevice, this->VKframeData[i].VkinFlightFences, nullptr);
            }
            
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

        //vkDeviceWaitIdle(this->VKdevice);
        state = false;

        return state;
    }

    void VulkanEngine::drawFrame()
    {
        // �������� �����ϱ� ���� �������� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        VkResult result = vkWaitForFences(this->VKdevice->VKdevice, 1, &this->getCurrnetFrameData().VkinFlightFences, VK_TRUE, UINT64_MAX);
        
        if (result != VK_SUCCESS) {
            return;
        }

        // �̹����� �������� ���� ���� ü�ο��� �̹��� �ε����� �����ɴϴ�.
        // �־��� ����ü�ο��� ���� �̹����� ȹ���ϰ�, 
        // ���������� ��������� �潺�� ����Ͽ� ����ȭ�� �����ϴ� Vulkan API�� �Լ��Դϴ�.
        uint32_t imageIndex;
        result = this->VKswapChain->acquireNextImage(this->getCurrnetFrameData().VkimageavailableSemaphore, imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            //this->recreateSwapChain(); // ���� ü���� �ٽ� �����մϴ�.
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // ���⼭ update�� ��, �����͸� �����Ų��. -> ��) uniform ���۸� ������Ʈ�մϴ�.
        //this->updateUniformBuffer(this->currentFrame);

        vkResetFences(this->VKdevice->VKdevice, 1, &this->getCurrnetFrameData().VkinFlightFences); // �÷��׸� �缳���մϴ�. -> �������� ������ �÷��׸� �缳���մϴ�.

        // �������� �����ϱ� ���� �̹����� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        // ������ ��� ���۸� �ʱ�ȭ�ϰ�, ���������� �÷��׸� ����Ͽ� �ʱ�ȭ ������ ����
        vkResetCommandBuffer(this->VKframeData[currentFrame].mainCommandBuffer, 0);
        
        this->recordCommandBuffer(&this->VKframeData[currentFrame], imageIndex);

        this->currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

        this->_isInitialized = true;
    }

    bool VulkanEngine::prepare()
    {
        this->init_commands();
        this->init_swapchain();
        this->init_sync_structures();
        this->createDepthStencilResources();
        this->createRenderPass();
        this->createPipelineCache();
        this->createFramebuffers();

        return true;
    }

    bool VulkanEngine::init_vulkan()
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

    bool VulkanEngine::init_commands()
    {
        // command pool create and command buffer create
        QueueFamilyIndices queueFamilyIndices = this->VKdevice->queueFamilyIndices;
        
        // Ŀ�ǵ� Ǯ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkCommandPoolCreateInfo poolInfo = helper::commandPoolCreateInfo(queueFamilyIndices.graphicsAndComputeFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        for (auto& framedata : this->VKframeData)
        {
            VkResult result = vkCreateCommandPool(this->VKdevice->VKdevice, &poolInfo, nullptr, &this->VKdevice->VKcommandPool);
            
            if (result != VK_SUCCESS) {
                return false;
            }

            VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(this->VKdevice->VKcommandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

            result = vkAllocateCommandBuffers(this->VKdevice->VKdevice, &allocInfo, &framedata.mainCommandBuffer);
            
            if (result != VK_SUCCESS) {
                return false;
            }

        }
        return true;
    }

    bool VulkanEngine::init_sync_structures()
    {
        VkSemaphoreCreateInfo semaphoreInfo = helper::semaphoreCreateInfo(0);
        VkFenceCreateInfo fenceInfo = helper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        for (auto& frameData : this->VKframeData)
        {
            VkResult result0 = vkCreateSemaphore(this->VKdevice->VKdevice, &semaphoreInfo, nullptr, &frameData.VkimageavailableSemaphore);
            VkResult result1 = vkCreateSemaphore(this->VKdevice->VKdevice, &semaphoreInfo, nullptr, &frameData.VkrenderFinishedSemaphore);
            VkResult result2 = vkCreateFence(this->VKdevice->VKdevice, &fenceInfo, nullptr, &frameData.VkinFlightFences);

            if (result0 != VK_SUCCESS || result1 != VK_SUCCESS || result2 != VK_SUCCESS) {
                return false;
            }
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
        // ���������� ���� ������� ����
        //helper::transitionImageLayout(
        //    this->VKdevice->VKdevice,
        //    this->VKcommandPool,
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
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
        subpass.pResolveAttachments = nullptr;
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

        if (vkCreatePipelineCache(this->VKdevice->VKdevice, &pipelineCacheCreateInfo, nullptr, &this->VKpipelineCache) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline cache!");
        }
    }

    void VulkanEngine::createFramebuffers()
    {
        this->VKswapChainFramebuffers.resize(this->VKswapChain->getSwapChainImages().size());

        for (size_t i = 0; i < this->VKswapChainFramebuffers.size(); i++)
        {
            std::array<VkImageView, 2> attachments = {
              this->VKswapChain->getSwapChainImageViews()[i],
              this->VKdepthStencill.depthImageView,
              // color attachment is resolved to this image
            };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = this->VKrenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = this->VKswapChain->getSwapChainExtent().width;
            framebufferInfo.height = this->VKswapChain->getSwapChainExtent().height;
            framebufferInfo.layers = 1;
            if (vkCreateFramebuffer(this->VKdevice->VKdevice, &framebufferInfo, nullptr, &this->VKswapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void VulkanEngine::recordCommandBuffer(FrameData *framedata, uint32_t imageIndex)
    {
        // Ŀ�ǵ� ���� ����� �����մϴ�.
        VkCommandBufferBeginInfo beginInfo = framedata->commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        if (vkBeginCommandBuffer(framedata->mainCommandBuffer, &beginInfo) != VK_SUCCESS) {
            return;
        }
        
        // ���� �н��� �����ϱ� ���� Ŭ���� �� ����
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        // ���� �н� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = this->VKrenderPass;
        renderPassInfo.framebuffer = this->VKswapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = this->VKswapChain->getSwapChainExtent();
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
        renderPassInfo.pClearValues = clearValues.data();

        // ���� �н��� �����մϴ�.
        vkCmdBeginRenderPass(framedata->mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            // �׷��� ������������ ���ε��մϴ�.
        //    vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);

        //    VkViewport viewport{};
        //    viewport.x = 0.0f;
        //    viewport.y = 0.0f;
        //    viewport.width = static_cast<float>(this->VKswapChainExtent.width);
        //    viewport.height = static_cast<float>(this->VKswapChainExtent.height);
        //    viewport.minDepth = 0.0f;
        //    viewport.maxDepth = 1.0f;
        //    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        //    VkRect2D scissor{};
        //    scissor.offset = { 0, 0 };
        //    scissor.extent = this->VKswapChainExtent;
        //    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        //    // ���ؽ� ���۸� ���ε��մϴ�.
        //    VkBuffer vertexBuffers[] = { this->VKvertexBuffer };
        //    VkDeviceSize offsets[] = { 0 };
        //    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        //    // �ε��� ���۸� ���ε��մϴ�.
        //    vkCmdBindIndexBuffer(commandBuffer, this->VKindexBuffer, 0, VK_INDEX_TYPE_UINT32);

        //    // ��ũ���� ��Ʈ�� ���ε��մϴ�.
        //    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKpipelineLayout, 0, 1, &this->VKdescriptorSets[currentFrame], 0, nullptr);
        //    // ���� �н��� �����մϴ�.
        //    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(this->VKindices.size()), 1, 0, 0, 0);
        }

        //vkCmdEndRenderPass(commandBuffer);

        // Ŀ�ǵ� ���� ����� �����մϴ�.
        //if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        //    throw std::runtime_error("failed to record command buffer!");
        //}
    }

    void VulkanEngine::cleanupSwapChain()
    {
        //for (auto swapChainView : this->VKswapChainImageViews)
        //{
        //    vkDestroyImageView(this->VKdevice->VKdevice, swapChainView, nullptr);
        //}
        //
        //vkDestroySwapchainKHR(this->VKdevice->VKdevice, this->VKswapChain, nullptr);
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


