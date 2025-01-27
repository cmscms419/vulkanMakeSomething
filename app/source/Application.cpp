
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
        glfwSetKeyCallback(this->VKwindow, vkutil::key_callback);  // Ű �Է� �ݹ� ����
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

        // 1. ���� ��ġ���� ť �йи� �ε����� ã���ϴ�.
        // 2. ť ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        // 3. ���� ��ġ ��� ����ü�� �ʱ�ȭ�մϴ�.
        // 4. �� ��ġ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        // 5. �� ��ġ�� �����մϴ�.
        
        // ť ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; // ����ü Ÿ���� �����մϴ�.
        queueCreateInfo.queueFamilyIndex = this->VKqueueFamilyIndices.getQueueFamilyIndex(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT); // �׷��Ƚ� ť �йи� �ε����� �����մϴ�.
        queueCreateInfo.queueCount = 1;                    // ť�� ������ �����մϴ�.

        float queuePriority = 1.0f;                        // ť�� �켱������ �����մϴ�.
        queueCreateInfo.pQueuePriorities = &queuePriority; // ť �켱���� �����͸� �����մϴ�.

        // ���� ��ġ ��� ����ü�� �ʱ�ȭ�մϴ�.
        VkPhysicalDeviceFeatures deviceFeatures{};

        // �� ��ġ ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;    // ����ü Ÿ���� �����մϴ�.
        createInfo.pQueueCreateInfos = &queueCreateInfo;            // ť ���� ���� �����͸� �����մϴ�.
        createInfo.queueCreateInfoCount = 1;                        // ť ���� ������ ������ �����մϴ�.
        createInfo.pEnabledFeatures = &deviceFeatures;              // ���� ��ġ ��� �����͸� �����մϴ�.
        createInfo.enabledExtensionCount = 0;                       // Ȱ��ȭ�� Ȯ�� ������ 0���� �����մϴ�.

        // �� ��ġ�� �����մϴ�.
        if (vkCreateDevice(this->VKphysicalDevice, &createInfo, nullptr, &this->VKdevice) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!"); // �� ��ġ ���� ���� �� ���ܸ� �߻���ŵ�ϴ�.
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

    // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ã�� �Լ�
    // PROB : ť �йи��� �������� ��쿡 �ʿ��� ó���� �ִ� �йи��� ���� ã�� ���, �� �йи��� �ε����� ��ȯ��
    // TODO ; ť �йи��� �������� ��쿡 ���� ó���� �ʿ���
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkQueueFlagBits bit)
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
            // �׷��Ƚ� ť�� �����ϴ� i° ť �йи��� �ε����� ��ȯ

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

        // �׷��Ƚ� ť�� �����ϴ� ť �йи��� ã�� ���� ��� 0�� ��ȯ
        return indices;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device, VkQueueFlagBits::VK_QUEUE_FLAG_BITS_MAX_ENUM);

        return false;
    }
}
