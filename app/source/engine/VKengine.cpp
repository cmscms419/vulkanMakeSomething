#include "VKengine.h"

using vkengine::input::key_callback;
using vkengine::VKdeviceHandler;
using vkengine::VKSwapChain;

using namespace vkengine::debug;

namespace vkengine
{
    constexpr cBool bUseValidationLayers = false;
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
        this->startTime = std::chrono::high_resolution_clock::now();
        this->currentTime = this->startTime;
    }

    VulkanEngine::~VulkanEngine() {}

    FrameData& VulkanEngine::getCurrnetFrameData()
    {
        // TODO: 여기에 return 문을 삽입합니다.
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

    cBool VulkanEngine::mainLoop()
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
        glfwSetKeyCallback(this->VKwindow, vkengine::input::key_callback);  // 키 입력 콜백 설정
        glfwSetCursorPosCallback(this->VKwindow, vkengine::input::cursorPositionCallback); // 마우스 입력 콜백 설정
        glfwSetMouseButtonCallback(this->VKwindow, vkengine::input::mouseButtonCallback); // 마우스 버튼 콜백 설정
        glfwSetScrollCallback(this->VKwindow, vkengine::input::scroll_callback); // 스크롤 콜백 설정
    }

    cBool VulkanEngine::prepare()
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
            this->recreateSwapChain(); // 스왑 체인을 다시 생성합니다.
            return;
        }
        else {
            _VK_CHECK_RESULT_(result);
        }
    }

    void VulkanEngine::presentFrame(uint32_t* imageIndex)
    {

        VkPresentInfoKHR presentInfo{};

        // VkPresentInfoKHR 구조체는 스왑 체인 이미지의 프레젠테이션을 위한 정보를 제공합니다.
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; // 구조체 타입을 지정합니다.
        presentInfo.pNext = nullptr;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &this->VKframeData[currentFrame].VkrenderFinishedSemaphore;

        VkSwapchainKHR swapChains[] = { this->VKswapChain->getSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pResults = nullptr;

        presentInfo.pImageIndices = imageIndex;

        // 프레젠테이션 큐에 이미지를 제출합니다.
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

    void VulkanEngine::createDescriptorPoolImGui()
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

    cBool VulkanEngine::initVulkan()
    {
        cBool check = false;

        if (glfwVulkanSupported() == GLFW_FALSE) {
            return check;
        }

        this->createInstance();

        if (enableValidationLayers)
        {
            this->setupDebugCallback();
        }

        check = this->createSurface();
        check = this->createDevice();

        return check;
    }

    cBool VulkanEngine::init_swapchain()
    {
        this->VKswapChain = std::make_unique<VKSwapChain>(this->VKdevice->physicalDevice, this->VKdevice->logicaldevice, this->VKsurface, &this->VKinstance);
        this->VKswapChain->createSwapChain(&this->VKdevice->queueFamilyIndices);
        this->VKswapChain->createImageViews();

        return true;
    }

    cBool VulkanEngine::init_command_pool()
    {
        // command pool create and command buffer create
        QueueFamilyIndices queueFamilyIndices = this->VKdevice->queueFamilyIndices;

        // 커맨드 풀 생성 정보 구조체를 초기화합니다.
        VkCommandPoolCreateInfo poolInfo = helper::commandPoolCreateInfo(queueFamilyIndices.graphicsAndComputeFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        _VK_CHECK_RESULT_(vkCreateCommandPool(this->VKdevice->logicaldevice, &poolInfo, nullptr, &this->VKdevice->commandPool));

        return true;
    }

    cBool VulkanEngine::init_sync_structures()
    {
        VkFenceCreateInfo fenceInfo = helper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        for (auto& frameData : this->VKframeData)
        {
            _VK_CHECK_RESULT_(vkCreateFence(this->VKdevice->logicaldevice, &fenceInfo, nullptr, &frameData.VkinFlightFences));
        }

        return true;
    }

    cBool VulkanEngine::createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // VkApplicationInfo 구조체는 애플리케이션에 대한 정보를 Vulkan에게 제공하기 위해 사용됩니다.
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;     // 구조체 타입을 지정합니다.
        appInfo.pApplicationName = "Vulkan egine";              // 애플리케이션 이름을 지정합니다.
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);  // 애플리케이션 버전을 지정합니다.
        appInfo.pEngineName = "vulkanEngine";                   // 엔진 이름을 지정합니다.
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // 엔진 버전을 지정합니다.
        appInfo.apiVersion = VK_API_VERSION_1_0;                // 사용할 Vulkan API 버전을 지정합니다.

        // VkInstanceCreateInfo 구조체는 Vulkan 인스턴스를 생성하기 위한 정보를 제공합니다.
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // 구조체 타입을 지정합니다.
        createInfo.pApplicationInfo = &appInfo;                    // VkApplicationInfo 구조체를 참조합니다.

        uint32_t glfwExtensionCount = 0;
        std::vector<const cChar*> extensions = this->getRequiredExtensions();     // 확장 목록을 저장할 변수를 선언합니다.
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
        _VK_CHECK_RESULT_(vkCreateInstance(&createInfo, nullptr, &this->VKinstance));
        _PRINT_TO_CONSOLE_("create instance\n");

        return true;
    }
    
    cBool VulkanEngine::setupDebugCallback()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        _VK_CHECK_RESULT_(CreateDebugUtilsMessengerEXT(this->VKinstance, &createInfo, nullptr, &this->VKdebugUtilsMessenger));

        return true;
    }
    
    cBool VulkanEngine::createSurface()
    {
#if CREATESURFACE_VKWIN32SURFACECREATEINFOKHR == 0
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = glfwGetWin32Window(this->VKwindow);
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(this->VKinstance, &createInfo, nullptr, &this->VKsurface) != VK_SUCCESS) {
            
            _PRINT_TO_CONSOLE_("failed to create window surface!");
            return false;
        }

        return true;
#else
        if (glfwCreateWindowSurface(this->VKinstance, this->VKwindow, nullptr, &this->VKsurface) != VK_SUCCESS) {
            _PRINT_TO_CONSOLE_("failed to create window surface!");
            return false;
        }
#endif
        //this->VKswapChain = new vkengine::VKSwapChain(this->VKdevice->VKphysicalDevice, this->VKdevice->VKdevice, this->VKsurface, &this->VKinstance);
    }
    
    cBool VulkanEngine::createDevice()
    {
        // 물리 디바이스 목록을 가져옵니다.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, nullptr);

        assert(deviceCount != 0);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, devices.data());

        // 물리 디바이스를 선택한다.
        // 먼저 물리 디바이스를 선택하기 위한 함수를 정의한다.
        // Score를 기준으로 가장 높은 점수의 물리 디바이스를 선택한다.
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
            _PRINT_TO_CONSOLE_("failed to find a suitable GPU!");
            return false;
        }

        this->VKdevice = std::make_unique<VKdeviceHandler>(pdevice, indices[selectQueueFamilyIndeices]);

        // 파생된 예제는 물리적 장치에서 읽은 지원되는 확장 목록에 따라 확장 기능을 활성화할 수 있습니다.
        // 필요할 때 코드 생성

        // 물리 장치 기능 구조체를 초기화합니다.
        // TODO: 나중에 다시 확인
        this->VKdevice->features.samplerAnisotropy = VK_TRUE; // 샘플러를 사용하여 텍스처를 보간합니다.
        this->VKdevice->features.sampleRateShading = VK_TRUE; // 샘플 레이트 쉐이딩을 사용하여 픽셀을 그립니다.

        // 논리 디바이스를 생성합니다.
        _VK_CHECK_RESULT_(this->VKdevice->createLogicalDevice());

        // depth format을 가져옵니다.
        this->VKdepthStencill.depthFormat = helper::findDepthFormat(this->VKdevice->physicalDevice);

        return true;
    }
    
    cBool VulkanEngine::createDepthStencilResources()
    {
        // 깊이 이미지 생성 정보 구조체를 초기화합니다.
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

        // 깊이 이미지 뷰를 생성합니다.
        this->VKdepthStencill.depthImageView = helper::createImageView(
            this->VKdevice->logicaldevice,
            this->VKdepthStencill.depthImage,
            this->VKdepthStencill.depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            1
        );

        // 깊이 이미지 레이아웃을 설정합니다.
        // 명시적으로 이미지 레이아웃을 전환합니다. -> 안정성을 위해서
        helper::transitionImageLayout(
            this->VKdevice->logicaldevice,
            this->VKdevice->commandPool,
            this->VKdevice->graphicsVKQueue,
            this->VKdepthStencill.depthImage,
            this->VKdepthStencill.depthFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            1
        );

        return true;
    }
    
    cBool VulkanEngine::createRenderPass()
    {
        // 렌더 패스 생성 정보 구조체를 초기화합니다.
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = this->VKswapChain->getSwapChainImageFormat();
        colorAttachment.samples = this->VKmsaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Render Pass 시작 시 클리어
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Render Pass 종료 시 저장
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // 사용하지 않음
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // 사용하지 않음
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 시작 레이아웃은 중요하지 않음.
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 프레젠테이션에 사용

        //// 색상 첨부 파일을 설정합니다.
        //VkAttachmentDescription colorAttachmentResolve{};
        //colorAttachmentResolve.format = this->VKswapChain->getSwapChainImageFormat();;
        //colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        //colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        //colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        //colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // 깊이 첨부 파일을 설정합니다.
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = this->VKdepthStencill.depthFormat;
        depthAttachment.samples = this->VKmsaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // 렌더 패스 서브패스를 설정합니다.
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 색상 첨부 파일 참조를 설정합니다.
        //VkAttachmentReference colorAttachmentResolveRef{};
        //colorAttachmentResolveRef.attachment = 2;
        //colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 깊이 첨부 파일 참조를 설정합니다.
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // 서브패스를 설정합니다.
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

        // 서브패스 종속성을 설정합니다.
        // 외부 서브패스와 대상 서브패스를 설정합니다.
        // 소스 스테이지 마스크와 대상 스테이지 마스크를 설정합니다.
        //이 코드는 외부(서브패스 미포함)에서 컬러·깊이/스텐실 단계가 끝날 때까지 기다린 후 
        // 메인 서브패스가 해당 리소스를 안전하게 쓰도록 동기화를 설정합니다.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 외부 서브패스
        dependency.dstSubpass = 0;                   // 대상 서브패스
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 소스 스테이지 마스크 -> 색상 첨부 출력 비트
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 대상 스테이지 마스크 -> 색상 첨부 출력 비트
        dependency.srcAccessMask = 0;                                                                                           // 소스 액세스 마스크 -> 0
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;         // 대상 액세스 마스크 -> 색상 첨부 쓰기 비트
        dependency.dependencyFlags = 0;                                                                                         // 종속성 플래그 -> 0

        // 첨부 파일을 설정합니다.
        std::array<VkAttachmentDescription, 2> dependencies = { colorAttachment, depthAttachment };

        // 렌더 패스 생성 정보 구조체를 초기화합니다.
        // 렌더 패스 생성 정보 구조체에 첨부 파일 및 서브패스를 설정합니다.
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pAttachments = dependencies.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        _VK_CHECK_RESULT_(vkCreateRenderPass(this->VKdevice->logicaldevice, &renderPassInfo, nullptr, this->VKrenderPass.get()));

        return true;
    }
    
    cBool VulkanEngine::createPipelineCache()
    {
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        _VK_CHECK_RESULT_(vkCreatePipelineCache(this->VKdevice->logicaldevice, &pipelineCacheCreateInfo, nullptr, &this->VKpipelineCache));

        return true;
    }
    
    cBool VulkanEngine::createFramebuffers()
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

        return true;
    }
    
    cBool VulkanEngine::recreateSwapChain()
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
        this->VKswapChain->createSwapChain(&this->VKdevice->queueFamilyIndices);  // 스왑 체인을 생성합니다.
        this->VKswapChain->createImageViews(); // 이미지 뷰를 생성합니다.
        VulkanEngine::createDepthStencilResources(); // 깊이 스텐실 리소스를 생성합니다.
        this->createFramebuffers(); // 렌더 패스를 생성합니다.

        return true;
    }

    cBool VulkanEngine::createCommandBuffer()
    {

#if 0
        for (auto& framedata : this->VKframeData)
        {
            framedata.mainCommandBuffer = this->VKdevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);
        }
#else
        VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(this->VKdevice->commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        
        for (auto& framedata : this->VKframeData)
        {
            _VK_CHECK_RESULT_(vkAllocateCommandBuffers(this->VKdevice->logicaldevice, &allocInfo, &framedata.mainCommandBuffer));
        }
#endif

        return true;
    }

    void VulkanEngine::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
    {

    }

    cBool VulkanEngine::checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const cChar* layerName : validationLayers) {
            cBool layerFound = false;

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

    std::vector<const cChar*> VulkanEngine::getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const cChar** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const cChar*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

}


