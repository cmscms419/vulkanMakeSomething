#include "VKswapchain.h"

using namespace vkengine::Log;

namespace vkengine
{
    VKSwapChain::VKSwapChain(VKcontext& ctx, VkSurfaceKHR surface, VkExtent2D& windowSize, bool vsync)
        : ctx(ctx), surface(surface), windowSize(windowSize)
    {
        this->initSurface(surface, vsync);
        this->createSwapChain();
    }

    void VKSwapChain::initSurface(VkSurfaceKHR surface, cBool vsync)
    {
        constexpr uint32_t INVALID_INDEX = UINT32_MAX;
        this->surface = surface;

        SwapChainSupportDetails swapChainSupport = helper::querySwapChainSupport(this->ctx.getDevice()->physicalDevice, this->surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, vsync);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
        this->imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        this->selectPresentMode = presentMode;
        this->colorSpace = surfaceFormat.colorSpace;
        this->ImageFormat = surfaceFormat.format;
        this->windowSize = extent;
        this->capabilities = swapChainSupport.capabilities;
    }

    void VKSwapChain::createSwapChain()
    {
        QueueFamilyIndices2 VKqueueFamilyIndices = this->ctx.getDevice()->queueFamilyIndices;

        cUint32_t graphicsQueueIndex = VKqueueFamilyIndices.grapicFamily;
        cUint32_t presentQueueindex = UINT32_MAX; // 프레젠트 큐 인덱스 초기화

        cUint32_t queueCount = static_cast<cUint32_t>(ctx.getDevice()->queueFamilyProperties.size());
        std::vector<VkBool32> supportsPresent(queueCount);
        for (uint32_t i = 0; i < queueCount; i++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(ctx.getDevice()->physicalDevice, i, this->surface,
                &supportsPresent[i]);
        }

        // 프레젠트 큐와 그래픽 큐가 동일한지 확인
        if (supportsPresent[graphicsQueueIndex] == VK_TRUE) {
            presentQueueindex = graphicsQueueIndex;
        }
        else {
            // Find any queue that supports presentation
            for (uint32_t i = 0; i < queueCount; ++i) {
                if (supportsPresent[i] == VK_TRUE) {
                    presentQueueindex = i;
                    break;
                }
            }
        }

        // 스왑 체인 생성 정보 구조체를 초기화합니다.
        VkSwapchainCreateInfoKHR createInfo{};

        // 큐 패밀리 인덱스를 가져옵니다.
        cUint32_t queueFamilyIndices[] = { graphicsQueueIndex, presentQueueindex };

        if (graphicsQueueIndex == UINT32_MAX || presentQueueindex == UINT32_MAX) {
            EXIT_TO_LOGGER("Failed to find graphics or present queue family index for swap chain");
        }

        // 여러 큐 패밀리에 걸쳐 사용될 스왑 체인 이미지를 처리하는 방법을 지정
        if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // 동시 공유 모드를 설정합니다.
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;

            PRINT_TO_LOGGER("Warning: graphics and present queue family indices are different.");
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // 배타적 공유 모드를 설정합니다.
            createInfo.queueFamilyIndexCount = 0;                     // Optional
        }

        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = this->surface;                                       // 스왑 체인이 연결될 표면을 설정합니다.
        createInfo.minImageCount = imageCount;                                    // 이미지 개수를 설정합니다.
        createInfo.imageFormat = this->ImageFormat;                            // 이미지 형식을 설정합니다.
        createInfo.imageColorSpace = this->colorSpace;                    // 이미지 색상 공간을 설정합니다.
        createInfo.imageExtent = this->windowSize;                                          // 이미지 해상도를 설정합니다.
        createInfo.imageArrayLayers = 1;                                          // 이미지 배열 레이어를 설정합니다.
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;              // 이미지 사용 방법을 설정합니다.
        createInfo.preTransform = this->capabilities.currentTransform; // 이미지 변환을 설정합니다.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;            // 알파 블렌딩을 설정합니다.
        createInfo.presentMode = this->selectPresentMode;                                     // 프레젠테이션 모드를 설정합니다.
        createInfo.clipped = VK_TRUE;                                             // 클리핑을 설정합니다. -> 다른 창이 앞에 있기 때문에 가려진 픽셀의 색상을 신경 쓰지 않는다는 의미
        createInfo.oldSwapchain = swapChain;                                      // 이전 스왑 체인을 설정합니다.

        _VK_CHECK_RESULT_(vkCreateSwapchainKHR(this->ctx.getDevice()->logicaldevice, &createInfo, nullptr, &this->swapChain));

        vkGetSwapchainImagesKHR(this->ctx.getDevice()->logicaldevice, this->swapChain, &imageCount, nullptr);
        this->Images.reserve(imageCount);
        this->Images.resize(imageCount);
        vkGetSwapchainImagesKHR(this->ctx.getDevice()->logicaldevice, this->swapChain, &imageCount, this->Images.data());

        this->createImageViews();

        barrierHelpers.reserve(this->imageCount);
        for (cUint32_t i = 0; i < this->imageCount; i++)
        {
            barrierHelpers.emplace_back(this->Images[i]);
            barrierHelpers[i].Format() = this->ImageFormat;
            barrierHelpers[i].MipLevels() = 1;   // Swapchain images have 1 mip level
            barrierHelpers[i].ArrayLayers() = 1; // Swapchain images have 1 array layer
            barrierHelpers[i].Currentlayout() = VK_IMAGE_LAYOUT_UNDEFINED;
            barrierHelpers[i].Currentaccess() = VK_ACCESS_2_NONE;
            barrierHelpers[i].Currentstage() = VK_PIPELINE_STAGE_2_NONE;
        }

        PRINT_TO_LOGGER("create swap chain\n");
        PRINT_TO_LOGGER("SwapChain imageCount: %d\n", imageCount);
        PRINT_TO_LOGGER("SwapChain imageFormat: %d\n", this->ImageFormat);
        PRINT_TO_LOGGER("SwapChain imageColorSpace: %d\n", this->colorSpace);
        PRINT_TO_LOGGER("SwapChain imageExtent.width: %d\n", this->windowSize.width);
        PRINT_TO_LOGGER("SwapChain imageExtent.height: %d\n", this->windowSize.height);
        PRINT_TO_LOGGER("SwapChain imageArrayLayers: %d\n", 1);
        PRINT_TO_LOGGER("SwapChain imageUsage: %d\n", createInfo.imageUsage);
        PRINT_TO_LOGGER("SwapChain imageSharingMode: %d\n", createInfo.imageSharingMode);
        PRINT_TO_LOGGER("SwapChain preTransform: %d\n", this->capabilities.currentTransform);
        PRINT_TO_LOGGER("SwapChain compositeAlpha: %d\n", createInfo.compositeAlpha);
        PRINT_TO_LOGGER("SwapChain presentMode: %d\n", this->selectPresentMode);
        PRINT_TO_LOGGER("SwapChain clipped: %d\n", VK_TRUE);
        PRINT_TO_LOGGER("\n");
    }

    void VKSwapChain::createImageViews()
    {
        this->ImageViews.resize(this->Images.size());

        for (int8_t i = 0; i < this->Images.size(); i++)
        {
            this->ImageViews[i] = helper::createImageView(
                this->ctx.getDevice()->logicaldevice,
                this->Images[i],
                this->ImageFormat,
                VK_IMAGE_ASPECT_COLOR_BIT,
                1);
        }
    }

    VkResult VKSwapChain::queuePresent(VkQueue queue, cUint32_t imageIndex, VkSemaphore waitSemaphore)
    {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = NULL;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &this->swapChain;
        presentInfo.pImageIndices = &imageIndex;

        if (waitSemaphore != VK_NULL_HANDLE) {
            presentInfo.pWaitSemaphores = &waitSemaphore;
            presentInfo.waitSemaphoreCount = 1;
        }
        return vkQueuePresentKHR(queue, &presentInfo);
    }

    void VKSwapChain::cleanup()
    {
        VkDevice device = this->ctx.getDevice()->logicaldevice;
        VkInstance instance = this->ctx.getInstance();

        if (device != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(device);
        }

        for (auto imageView : this->ImageViews) {
            vkDestroyImageView(this->ctx.getDevice()->logicaldevice, imageView, nullptr);
        }

        this->ImageViews.clear();

        if (this->swapChain != VK_NULL_HANDLE){
            vkDestroySwapchainKHR(this->ctx.getDevice()->logicaldevice, this->swapChain, nullptr);
            this->swapChain = VK_NULL_HANDLE;
        }

        this->Images.clear();
        /* this->barierHelper.clear(); */

        if (this->surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, this->surface, nullptr);
            this->surface = VK_NULL_HANDLE;
        }

        this->barrierHelpers.clear();

        this->ImageFormat = VK_FORMAT_UNDEFINED;
        this->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        this->imageCount = 0;
    }

    // 서피스 포맷을 선택하는 함수
    // SRGB 색상 공간을 지원하는 32비트 BGR 색상 구조를 선택
    // 이러한 형식이 지원되지 않으면 첫 번째 형식을 반환
    // 이러한 형식이 없으면 예외를 발생시킵니다.
    VkSurfaceFormatKHR VKSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        VkSurfaceFormatKHR targetformat = {};

        std::vector<VkFormat> preferredImageFormats = { VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB,
                                              VK_FORMAT_B8G8R8A8_UNORM,
                                              VK_FORMAT_B8G8R8A8_SRGB,
                                              VK_FORMAT_A8B8G8R8_UNORM_PACK32 };

        for (const auto& availableFormat : availableFormats)
        {
            for (const auto& preferredFormat : preferredImageFormats)
            {
                if (availableFormat.format == preferredFormat &&
                    availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    targetformat = availableFormat;
                    break;
                }
            }
            if (targetformat.format != VK_FORMAT_UNDEFINED) {
                break;
            }
        }

        if (targetformat.format == VK_FORMAT_UNDEFINED)
        {
            EXIT_TO_LOGGER("Failed to find a suitable surface format for swap chain");
        }

        return targetformat;
    }

    // 프레젠테이션 모드를 선택하는 함수
    // VK_PRESENT_MODE_MAILBOX_KHR 프레젠테이션 모드를 지원하는지 확인
    // 지원되는 경우 VK_PRESENT_MODE_MAILBOX_KHR를 반환
    VkPresentModeKHR VKSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, cBool vsync)
    {
        // 기본 프레젠테이션 모드는 FIFO 모드로 설정
        VkPresentModeKHR targetPresentMode = VK_PRESENT_MODE_FIFO_KHR;

        // vsync이 활성화된 경우 VK_PRESENT_MODE_MAILBOX_KHR 모드를 우선적으로 선택
        if (vsync) {
            // 고성능 모드 
            for (const auto& availablePresentMode : availablePresentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    targetPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                }
            }
        }
        else
        {
            for (const auto& availablePresentMode : availablePresentModes) {
                // 즉시 제시 모드
                if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                    targetPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                    break;
                }
                else if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    targetPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                }
            }

        }
        return targetPresentMode;
    }

    // 스왑 체인 이미지의 해상도를 선택합니다.
    // common_.h 에 정의된 크기를 기준으로 해상도를 설정합니다.
    // 해상도는 최소 및 최대 이미지 해상도 사이에서 클램프됩니다.
    VkExtent2D VKSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        if (this->windowSize.width != 0 && this->windowSize.height != 0) {
            VkExtent2D actualExtent = this->windowSize;
            actualExtent.width = glm::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = glm::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
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
