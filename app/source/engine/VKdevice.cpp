#include "VKdevice.h"
#include "helper.h"

namespace vkengine {

    using namespace helper;

    VKDevice_::VKDevice_(VkPhysicalDevice physicalDevice, QueueFamilyIndices indice)
    {
        assert(physicalDevice);
        assert(indice.isComplete());

        this->VKphysicalDevice = physicalDevice;
        this->queueFamilyIndices = indice;

        vkGetPhysicalDeviceProperties(this->VKphysicalDevice, &properties);       // 물리 디바이스 속성 가져오기
        vkGetPhysicalDeviceFeatures(this->VKphysicalDevice, &features);           // 물리 디바이스 기능 가져오기
        vkGetPhysicalDeviceMemoryProperties(VKphysicalDevice, &memoryProperties); // 메모리 속성 가져오기

        uint32_t supportedApiVersion = properties.apiVersion;

        if (supportedApiVersion >= VK_API_VERSION_1_2) {
            // Vulkan 1.2 이상을 지원하는 디바이스 처리
            // 예: 새로운 기능 활성화
        }
        else if (supportedApiVersion >= VK_API_VERSION_1_1) {
            // Vulkan 1.1을 지원하는 디바이스 처리
        }
        else {
            // Vulkan 1.0을 지원하는 디바이스 처리
        }

#ifdef DEBUG_
        printf("Select Device\n");
        printf("Select DeviceProperties.deviceType: %d\n", properties.deviceType);
        printf("Select Device Name: %s\n", properties.deviceName);
#endif // DEBUG_

        helper::getDeviceExtensionSupport(physicalDevice, &supportedExtensions);
    }

    VkResult VKDevice_::createLogicalDevice()
    {
        // 1. 물리 장치에서 큐 패밀리 인덱스를 찾습니다.
        // 2. 큐 생성 정보 구조체를 초기화합니다.
        // 3. 물리 장치 기능 구조체를 초기화합니다.
        // 4. 논리 장치 생성 정보 구조체를 초기화합니다.
        // 5. 논리 장치를 생성합니다.
        VkResult result = VK_SUCCESS;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::set<uint32_t> uniqueQueueFamilies = {
            this->queueFamilyIndices.graphicsAndComputeFamily,
            this->queueFamilyIndices.presentFamily
        };

        float queuePriority = 1.0f;                                                                    // 큐의 우선순위를 설정합니다.
        
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;                        // 구조체 타입을 지정합니다.
            queueCreateInfo.queueFamilyIndex = queueFamily;                                            // 그래픽스 큐 패밀리 인덱스를 설정합니다.
            queueCreateInfo.queueCount = 1;                                                            // 큐의 개수를 설정합니다.
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 논리 장치 생성 정보 구조체를 초기화합니다.
        VkDeviceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;                            // 구조체 타입을 지정합니다.
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());   // 큐 생성 정보의 개수를 설정합니다.
        createInfo.pQueueCreateInfos = queueCreateInfos.data();                             // 큐 생성 정보 포인터를 설정합니다.
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());  // 활성화할 확장 개수를 설정합니다.
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();                       // 활성화할 확장 목록을 설정합니다.
        createInfo.pEnabledFeatures = &this->features;                                      // 물리 장치 기능 포인터를 설정합니다.

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // 논리 장치를 생성합니다.
        result = vkCreateDevice(this->VKphysicalDevice, &createInfo, nullptr, &this->VKdevice);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!"); // 논리 장치 생성 실패 시 예외를 발생시킵니다.
        }

        // 논리 디바이스에서 그래픽 큐 핸들을 가져옵니다.
        vkGetDeviceQueue(this->VKdevice, this->queueFamilyIndices.graphicsAndComputeFamily, 0, &this->graphicsVKQueue);
        
        // 논리 디바이스에서 프레젠테이션 큐 핸들을 가져옵니다.
        vkGetDeviceQueue(this->VKdevice, this->queueFamilyIndices.presentFamily, 0, &this->presentVKQueue);

        return result;
    }

    VKDevice_::~VKDevice_()
    {
    }
}