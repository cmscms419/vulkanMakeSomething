
#include "../../include/source/Application.h"
#include "../../include/source/DebugFunction.h"

using namespace vkutil;

namespace vkutil {

    Application::Application() {
        this->VKwindow = nullptr;
        this->VKinstance = {};
        this->VKdebugUtilsMessenger = VK_NULL_HANDLE;
        this->VKphysicalDevice = VK_NULL_HANDLE;
        this->VKqueueFamilyIndices.index = 0;
        this->VKqueueFamilyIndices.queueFamilyProperties = {};
        this->VKdevice = VK_NULL_HANDLE;
        this->VKQueue = VK_NULL_HANDLE;
        this->state = true;
    }

    Application::~Application()
    {
    }

    void Application::init() {
        initWindow();
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
        vkDestroyDevice(this->VKdevice, nullptr);
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
        this->pickPhysicalDevice();
        this->createLogicalDevice();
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
                this->VKqueueFamilyIndices = findQueueFamilies(candidate.second, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
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
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; // 구조체 타입을 지정합니다.
        queueCreateInfo.queueFamilyIndex = this->VKqueueFamilyIndices.getQueueFamilyIndex(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT); // 그래픽스 큐 패밀리 인덱스를 설정합니다.
        queueCreateInfo.queueCount = 1;                    // 큐의 개수를 설정합니다.

        float queuePriority = 1.0f;                        // 큐의 우선순위를 설정합니다.
        queueCreateInfo.pQueuePriorities = &queuePriority; // 큐 우선순위 포인터를 설정합니다.

        // 물리 장치 기능 구조체를 초기화합니다.
        VkPhysicalDeviceFeatures deviceFeatures{};

        // 논리 장치 생성 정보 구조체를 초기화합니다.
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;    // 구조체 타입을 지정합니다.
        createInfo.pQueueCreateInfos = &queueCreateInfo;            // 큐 생성 정보 포인터를 설정합니다.
        createInfo.queueCreateInfoCount = 1;                        // 큐 생성 정보의 개수를 설정합니다.
        createInfo.pEnabledFeatures = &deviceFeatures;              // 물리 장치 기능 포인터를 설정합니다.
        createInfo.enabledExtensionCount = 0;                       // 활성화할 확장 개수를 0으로 설정합니다.

        // 논리 장치를 생성합니다.
        if (vkCreateDevice(this->VKphysicalDevice, &createInfo, nullptr, &this->VKdevice) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!"); // 논리 장치 생성 실패 시 예외를 발생시킵니다.
        }

        vkGetDeviceQueue(this->VKdevice, this->VKqueueFamilyIndices.index, 0, &this->VKQueue);
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

    // 주어진 물리 장치에서 큐 패밀리 속성을 찾는 함수
    // PROB : 큐 패밀리가 여러개인 경우에 필요한 처리가 있는 패밀리를 먼저 찾을 경우, 그 패밀리의 인덱스만 반환함
    // TODO ; 큐 패밀리가 여러개인 경우에 대한 처리가 필요함
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkQueueFlagBits bit)
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
            // 그래픽스 큐를 지원하는 i째 큐 패밀리의 인덱스를 반환

#ifdef DEBUG_
            printf("QueueFamily %d\n", i);
            printf("QueueFamily queueCount: %d\n", queueFamily.queueCount);
            printf("QueueFamily queueFlags: %d\n", queueFamily.queueFlags);
            printf("QueueFamily timestampValidBits: %d\n", queueFamily.timestampValidBits);
            printf("QueueFamily minImageTransferGranularity.width: %d\n", queueFamily.minImageTransferGranularity.width);
            printf("QueueFamily minImageTransferGranularity.height: %d\n", queueFamily.minImageTransferGranularity.height);
            printf("QueueFamily minImageTransferGranularity.depth: %d\n", queueFamily.minImageTransferGranularity.depth);
#endif // DEBUG_

#if 0
            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) {
                printf("VK_QUEUE_GRAPHICS_BIT is supported\n");
                indices.graphicsFamily.push_back(i);
            }
            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
            {
                printf("VK_QUEUE_COMPUTE_BIT is supported\n");
                indices.graphicsFamily.push_back(i);
            }
            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
            {
                printf("VK_QUEUE_TRANSFER_BIT is supported\n");
                indices.graphicsFamily.push_back(i);
            }
            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_SPARSE_BINDING_BIT)
            {
                printf("VK_QUEUE_SPARSE_BINDING_BIT is supported\n");
                indices.graphicsFamily.push_back(i);
            }
            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_PROTECTED_BIT)
            {
                printf("VK_QUEUE_PROTECTED_BIT is supported\n");
                indices.graphicsFamily.push_back(i);
            }
            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR)
            {
                printf("VK_QUEUE_VIDEO_DECODE_BIT_KHR is supported\n");
                indices.graphicsFamily.push_back(i);
            }
            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
            {
                printf("VK_QUEUE_VIDEO_ENCODE_BIT_KHR is supported\n");
                indices.graphicsFamily.push_back(i);
            }
            if (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_OPTICAL_FLOW_BIT_NV)
            {
                printf("VK_QUEUE_OPTICAL_FLOW_BIT_NV is supported\n");
                indices.graphicsFamily.push_back(i);
            }
            printf("\n");
#endif

            if (queueFamily.queueFlags & bit) {
                switch (bit)
                {
                case VK_QUEUE_GRAPHICS_BIT:
                    printf("VK_QUEUE_GRAPHICS_BIT is supported\n");
                    break;
                case VK_QUEUE_COMPUTE_BIT:
                    printf("VK_QUEUE_COMPUTE_BIT is supported\n");
                    break;
                case VK_QUEUE_TRANSFER_BIT:
                    printf("VK_QUEUE_TRANSFER_BIT is supported\n");
                    break;
                case VK_QUEUE_SPARSE_BINDING_BIT:
                    printf("VK_QUEUE_SPARSE_BINDING_BIT is supported\n");
                    break;
                case VK_QUEUE_PROTECTED_BIT:
                    printf("VK_QUEUE_PROTECTED_BIT is supported\n");
                    break;
                case VK_QUEUE_VIDEO_DECODE_BIT_KHR:
                    printf("VK_QUEUE_VIDEO_DECODE_BIT_KHR is supported\n");
                    break;
                case VK_QUEUE_VIDEO_ENCODE_BIT_KHR:
                    printf("VK_QUEUE_VIDEO_ENCODE_BIT_KHR is supported\n");
                    break;
                case VK_QUEUE_OPTICAL_FLOW_BIT_NV:
                    printf("VK_QUEUE_OPTICAL_FLOW_BIT_NV is supported\n");
                    break;
                case VK_QUEUE_FLAG_BITS_MAX_ENUM:
                    printf("VK_QUEUE_FLAG_BITS_MAX_ENUM\n");
                    break;
                default:
                    break;
                }

                indices.index = i;
                indices.queueFamilyProperties = queueFamily;
                printf("Queuefamily index: %d\n", i);
                printf("\n");
                break;
            }
            else
            {
                printf("VK_QUEUE_GRAPHICS_BIT is not supported\n");
            }
            i++;
        }

        // 그래픽스 큐를 지원하는 큐 패밀리를 찾지 못한 경우 0을 반환
        return indices;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device, VkQueueFlagBits::VK_QUEUE_FLAG_BITS_MAX_ENUM);

        return false;
    }
}
