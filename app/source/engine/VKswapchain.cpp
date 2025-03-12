#include "VKswapchain.h"

namespace vkengine
{
    VKSwapChain::VKSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkInstance* Instance)
    {
        this->physicalDevice = physicalDevice;
        this->logicaldevice = device;
        this->surface = surface;
        this->Instance = *Instance;

    }

    void VKSwapChain::createSwapChain(QueueFamilyIndices* VKqueueFamilyIndices)
    {
        SwapChainSupportDetails swapChainSupport = helper::querySwapChainSupport(this->physicalDevice, this->surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // 스왑 체인 생성 정보 구조체를 초기화합니다.
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = this->surface;

        createInfo.minImageCount = imageCount;                          // 이미지 개수를 설정합니다.
        createInfo.imageFormat = surfaceFormat.format;                  // 이미지 형식을 설정합니다.
        createInfo.imageColorSpace = surfaceFormat.colorSpace;          // 이미지 색상 공간을 설정합니다.
        createInfo.imageExtent = extent;                                // 이미지 해상도를 설정합니다.
        createInfo.imageArrayLayers = 1;                                // 이미지 배열 레이어를 설정합니다.
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;    // 이미지 사용 방법을 설정합니다.

        // 큐 패밀리 인덱스를 가져옵니다.
        uint32_t queueFamilyIndices[] = { VKqueueFamilyIndices->graphicsAndComputeFamily, VKqueueFamilyIndices->presentFamily };

        // 여러 큐 패밀리에 걸쳐 사용될 스왑 체인 이미지를 처리하는 방법을 지정
        if (VKqueueFamilyIndices->graphicsAndComputeFamily != VKqueueFamilyIndices->presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // 동시 공유 모드를 설정합니다.
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // 배타적 공유 모드를 설정합니다.
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // 이미지 변환을 설정합니다.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;            // 알파 블렌딩을 설정합니다.
        createInfo.presentMode = presentMode;                                     // 프레젠테이션 모드를 설정합니다.
        createInfo.clipped = VK_TRUE;                                             // 클리핑을 설정합니다. -> 다른 창이 앞에 있기 때문에 가려진 픽셀의 색상을 신경 쓰지 않는다는 의미
        createInfo.oldSwapchain = VK_NULL_HANDLE;                                 // 이전 스왑 체인을 설정합니다. -> 나중에 설정

        _VK_CHECK_RESULT_(vkCreateSwapchainKHR(this->logicaldevice, &createInfo, nullptr, &this->swapChain));

        vkGetSwapchainImagesKHR(this->logicaldevice, this->swapChain, &imageCount, nullptr);
        this->Images.resize(imageCount);
        vkGetSwapchainImagesKHR(this->logicaldevice, this->swapChain, &imageCount, this->Images.data());

        this->ImageFormat = surfaceFormat.format;
        this->Extent = extent;

        _PRINT_TO_CONSOLE_("create swap chain\n");
        _PRINT_TO_CONSOLE_("SwapChain imageCount: %d\n", imageCount);
        _PRINT_TO_CONSOLE_("SwapChain imageFormat: %d\n", surfaceFormat.format);
        _PRINT_TO_CONSOLE_("SwapChain imageColorSpace: %d\n", surfaceFormat.colorSpace);
        _PRINT_TO_CONSOLE_("SwapChain imageExtent.width: %d\n", extent.width);
        _PRINT_TO_CONSOLE_("SwapChain imageExtent.height: %d\n", extent.height);
        _PRINT_TO_CONSOLE_("SwapChain imageArrayLayers: %d\n", 1);
        _PRINT_TO_CONSOLE_("SwapChain imageUsage: %d\n", VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        _PRINT_TO_CONSOLE_("SwapChain imageSharingMode: %d\n", VK_SHARING_MODE_EXCLUSIVE);
        _PRINT_TO_CONSOLE_("SwapChain preTransform: %d\n", swapChainSupport.capabilities.currentTransform);
        _PRINT_TO_CONSOLE_("SwapChain compositeAlpha: %d\n", VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
        _PRINT_TO_CONSOLE_("SwapChain presentMode: %d\n", presentMode);
        _PRINT_TO_CONSOLE_("SwapChain clipped: %d\n", VK_TRUE);
        _PRINT_TO_CONSOLE_("\n");

    }

    void VKSwapChain::cleanupSwapChain()
    {
        for (auto imageView : this->ImageViews) {
            vkDestroyImageView(this->logicaldevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(this->logicaldevice, this->swapChain, nullptr);
    }

    void VKSwapChain::createImageViews()
    {
        this->ImageViews.resize(this->Images.size());

        for (int8_t i = 0; i < this->Images.size(); i++)
        {
            this->ImageViews[i] = helper::createImageView(
                this->logicaldevice,
                this->Images[i],
                this->ImageFormat,
                VK_IMAGE_ASPECT_COLOR_BIT,
                1);
        }
    }

    // 서피스 포맷을 선택하는 함수
    // SRGB 색상 공간을 지원하는 32비트 BGR 색상 구조를 선택
    // 이러한 형식이 지원되지 않으면 첫 번째 형식을 반환
    // 이러한 형식이 없으면 예외를 발생시킵니다.
    VkSurfaceFormatKHR VKSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        VkSurfaceFormatKHR targetformat = {};

        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&            // 32비트 BGR 색상 구조를 지원하는지 확인
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR // SRGB 색상 공간을 지원하는지 확인
                )
            {
                targetformat = availableFormat;
                break;
            }
        }

        return targetformat;
    }

    // 프레젠테이션 모드를 선택하는 함수
    // VK_PRESENT_MODE_MAILBOX_KHR 프레젠테이션 모드를 지원하는지 확인
    // 지원되는 경우 VK_PRESENT_MODE_MAILBOX_KHR를 반환
    VkPresentModeKHR VKSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // 스왑 체인 이미지의 해상도를 선택합니다.
    // common_.h 에 정의된 크기를 기준으로 해상도를 설정합니다.
    // 해상도는 최소 및 최대 이미지 해상도 사이에서 클램프됩니다.
    VkExtent2D VKSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width = WIDTH, height = HEIGHT;

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }

    }

}
