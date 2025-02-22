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
        // 렌더링을 시작하기 전에 프레임을 렌더링할 준비가 되었는지 확인합니다.
        VkResult result = vkWaitForFences(this->VKdevice->VKdevice, 1, &this->getCurrnetFrameData().VkinFlightFences, VK_TRUE, UINT64_MAX);
        
        if (result != VK_SUCCESS) {
            return;
        }

        // 이미지를 가져오기 위해 스왑 체인에서 이미지 인덱스를 가져옵니다.
        // 주어진 스왑체인에서 다음 이미지를 획득하고, 
        // 선택적으로 세마포어와 펜스를 사용하여 동기화를 관리하는 Vulkan API의 함수입니다.
        uint32_t imageIndex;
        result = this->VKswapChain->acquireNextImage(this->getCurrnetFrameData().VkimageavailableSemaphore, imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            //this->recreateSwapChain(); // 스왑 체인을 다시 생성합니다.
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // 여기서 update를 된, 데이터를 적용시킨다. -> 예) uniform 버퍼를 업데이트합니다.
        //this->updateUniformBuffer(this->currentFrame);

        vkResetFences(this->VKdevice->VKdevice, 1, &this->getCurrnetFrameData().VkinFlightFences); // 플래그를 재설정합니다. -> 렌더링이 끝나면 플래그를 재설정합니다.

        // 렌더링을 시작하기 전에 이미지를 렌더링할 준비가 되었는지 확인합니다.
        // 지정된 명령 버퍼를 초기화하고, 선택적으로 플래그를 사용하여 초기화 동작을 제어
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
        glfwSetKeyCallback(this->VKwindow, vkengine::input::key_callback);  // 키 입력 콜백 설정

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
        
        // 커맨드 풀 생성 정보 구조체를 초기화합니다.
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
        std::vector<const char*> extensions = this->getRequiredExtensions();     // 확장 목록을 저장할 변수를 선언합니다.
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
        if (vkCreateInstance(&createInfo, nullptr, &this->VKinstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!"); // 인스턴스 생성 실패 시 예외를 발생시킵니다.
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
        // 물리 디바이스 목록을 가져옵니다.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, devices.data());

        // 물리 디바이스를 선택한다.
        // 먼저 물리 디바이스를 선택하기 위한 함수를 정의한다.
        // Score를 기준으로 가장 높은 점수의 물리 디바이스를 선택한다.
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

        // 파생된 예제는 물리적 장치에서 읽은 지원되는 확장 목록에 따라 확장 기능을 활성화할 수 있습니다.
        // 필요할 때 코드 생성

        // 물리 장치 기능 구조체를 초기화합니다.
        this->VKdevice->features.samplerAnisotropy = VK_TRUE; // 샘플러를 사용하여 텍스처를 보간합니다.
        this->VKdevice->features.sampleRateShading = VK_TRUE; // 샘플 레이트 쉐이딩을 사용하여 픽셀을 그립니다.

        // 논리 디바이스를 생성합니다.
        VkResult result = this->VKdevice->createLogicalDevice();

        // depth format을 가져옵니다.
        this->VKdepthStencill.depthFormat = helper::findDepthFormat(this->VKdevice->VKphysicalDevice);
    }

    void VulkanEngine::createDepthStencilResources()
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
            this->VKdevice->VKdevice,
            this->VKdepthStencill.depthImage,
            this->VKdepthStencill.depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            1
        );

        // 깊이 이미지 레이아웃을 설정합니다.
        // 예제에서는 아직 사용하지 않음
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
        // 렌더 패스 생성 정보 구조체를 초기화합니다.
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = this->VKswapChain->getSwapChainImageFormat();
        colorAttachment.samples = this->VKmsaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
        subpass.pResolveAttachments = nullptr;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        // 서브패스 종속성을 설정합니다.
        // 외부 서브패스와 대상 서브패스를 설정합니다.
        // 소스 스테이지 마스크와 대상 스테이지 마스크를 설정합니다.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 외부 서브패스
        dependency.dstSubpass = 0;                   // 대상 서브패스
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 소스 스테이지 마스크 -> 색상 첨부 출력 비트
        dependency.srcAccessMask = 0;                                                                                           // 소스 액세스 마스크 -> 0
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;   // 대상 스테이지 마스크 -> 색상 첨부 출력 비트
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;         // 대상 액세스 마스크 -> 색상 첨부 쓰기 비트

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
        // 커맨드 버퍼 기록을 시작합니다.
        VkCommandBufferBeginInfo beginInfo = framedata->commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        if (vkBeginCommandBuffer(framedata->mainCommandBuffer, &beginInfo) != VK_SUCCESS) {
            return;
        }
        
        // 렌더 패스를 시작하기 위한 클리어 값 설정
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        // 렌더 패스 시작 정보 구조체를 초기화합니다.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = this->VKrenderPass;
        renderPassInfo.framebuffer = this->VKswapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = this->VKswapChain->getSwapChainExtent();
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
        renderPassInfo.pClearValues = clearValues.data();

        // 렌더 패스를 시작합니다.
        vkCmdBeginRenderPass(framedata->mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            // 그래픽 파이프라인을 바인딩합니다.
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

        //    // 버텍스 버퍼를 바인딩합니다.
        //    VkBuffer vertexBuffers[] = { this->VKvertexBuffer };
        //    VkDeviceSize offsets[] = { 0 };
        //    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        //    // 인덱스 버퍼를 바인딩합니다.
        //    vkCmdBindIndexBuffer(commandBuffer, this->VKindexBuffer, 0, VK_INDEX_TYPE_UINT32);

        //    // 디스크립터 세트를 바인딩합니다.
        //    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKpipelineLayout, 0, 1, &this->VKdescriptorSets[currentFrame], 0, nullptr);
        //    // 렌더 패스를 종료합니다.
        //    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(this->VKindices.size()), 1, 0, 0, 0);
        }

        //vkCmdEndRenderPass(commandBuffer);

        // 커맨드 버퍼 기록을 종료합니다.
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


