#include "VKswapchain.h"
#include "helper.h"

namespace vkengine
{
    VKSwapChain::VKSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkInstance* Instance)
    {
        this->VKphysicalDevice = physicalDevice;
        this->VKdevice = device;
        this->VKsurface = surface;
        this->VKInstance = *Instance;

    }

    VKSwapChain::~VKSwapChain()
    {
        this->cleanupSwapChain();
    }

    void VKSwapChain::createSwapChain(QueueFamilyIndices* VKqueueFamilyIndices)
    {
        SwapChainSupportDetails swapChainSupport = helper::querySwapChainSupport(this->VKphysicalDevice, this->VKsurface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // ���� ü�� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = this->VKsurface;

        createInfo.minImageCount = imageCount;                          // �̹��� ������ �����մϴ�.
        createInfo.imageFormat = surfaceFormat.format;                  // �̹��� ������ �����մϴ�.
        createInfo.imageColorSpace = surfaceFormat.colorSpace;          // �̹��� ���� ������ �����մϴ�.
        createInfo.imageExtent = extent;                                // �̹��� �ػ󵵸� �����մϴ�.
        createInfo.imageArrayLayers = 1;                                // �̹��� �迭 ���̾ �����մϴ�.
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;    // �̹��� ��� ����� �����մϴ�.

        // ť �йи� �ε����� �����ɴϴ�.
        uint32_t queueFamilyIndices[] = { VKqueueFamilyIndices->graphicsAndComputeFamily, VKqueueFamilyIndices->presentFamily };

        // ���� ť �йи��� ���� ���� ���� ü�� �̹����� ó���ϴ� ����� ����
        if (VKqueueFamilyIndices->graphicsAndComputeFamily != VKqueueFamilyIndices->presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // ���� ���� ��带 �����մϴ�.
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // ��Ÿ�� ���� ��带 �����մϴ�.
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // �̹��� ��ȯ�� �����մϴ�.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;            // ���� ������ �����մϴ�.
        createInfo.presentMode = presentMode;                                     // ���������̼� ��带 �����մϴ�.
        createInfo.clipped = VK_TRUE;                                             // Ŭ������ �����մϴ�. -> �ٸ� â�� �տ� �ֱ� ������ ������ �ȼ��� ������ �Ű� ���� �ʴ´ٴ� �ǹ�
        createInfo.oldSwapchain = VK_NULL_HANDLE;                                 // ���� ���� ü���� �����մϴ�. -> ���߿� ����

        if (vkCreateSwapchainKHR(this->VKdevice, &createInfo, nullptr, &this->VKswapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(this->VKdevice, this->VKswapChain, &imageCount, nullptr);
        this->VKswapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(this->VKdevice, this->VKswapChain, &imageCount, this->VKswapChainImages.data());

        this->VKswapChainImageFormat = surfaceFormat.format;
        this->VKswapChainExtent = extent;

#ifdef DEBUG_
        printf("create swap chain\n");
        printf("SwapChain imageCount: %d\n", imageCount);
        printf("SwapChain imageFormat: %d\n", surfaceFormat.format);
        printf("SwapChain imageColorSpace: %d\n", surfaceFormat.colorSpace);
        printf("SwapChain imageExtent.width: %d\n", extent.width);
        printf("SwapChain imageExtent.height: %d\n", extent.height);
        printf("SwapChain imageArrayLayers: %d\n", 1);
        printf("SwapChain imageUsage: %d\n", VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        printf("SwapChain imageSharingMode: %d\n", VK_SHARING_MODE_EXCLUSIVE);
        printf("SwapChain preTransform: %d\n", swapChainSupport.capabilities.currentTransform);
        printf("SwapChain compositeAlpha: %d\n", VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
        printf("SwapChain presentMode: %d\n", presentMode);
        printf("SwapChain clipped: %d\n", VK_TRUE);
        printf("\n");
#endif // DEBUG_

    }

    VkResult VKSwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex)
    {
        return vkAcquireNextImageKHR(this->VKdevice, this->VKswapChain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, &imageIndex);
    }

    void VKSwapChain::cleanupSwapChain()
    {

        for (auto imageView : this->VKswapChainImageViews) {
            vkDestroyImageView(this->VKdevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(this->VKdevice, this->VKswapChain, nullptr);
    }

    void VKSwapChain::createImageViews()
    {
        this->VKswapChainImageViews.resize(this->VKswapChainImages.size());

        for (int8_t i = 0; i < this->VKswapChainImages.size(); i++)
        {
            this->VKswapChainImageViews[i] = helper::createImageView(
                this->VKdevice,
                this->VKswapChainImages[i],
                this->VKswapChainImageFormat,
                VK_IMAGE_ASPECT_COLOR_BIT,
                1);
        }
    }

    // ���ǽ� ������ �����ϴ� �Լ�
    // SRGB ���� ������ �����ϴ� 32��Ʈ BGR ���� ������ ����
    // �̷��� ������ �������� ������ ù ��° ������ ��ȯ
    // �̷��� ������ ������ ���ܸ� �߻���ŵ�ϴ�.
    VkSurfaceFormatKHR VKSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        VkSurfaceFormatKHR targetformat = {};

        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&            // 32��Ʈ BGR ���� ������ �����ϴ��� Ȯ��
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR // SRGB ���� ������ �����ϴ��� Ȯ��
                )
            {
                targetformat = availableFormat;
                break;
            }
        }

        return targetformat;
    }

    // ���������̼� ��带 �����ϴ� �Լ�
    // VK_PRESENT_MODE_MAILBOX_KHR ���������̼� ��带 �����ϴ��� Ȯ��
    // �����Ǵ� ��� VK_PRESENT_MODE_MAILBOX_KHR�� ��ȯ
    VkPresentModeKHR VKSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // ���� ü�� �̹����� �ػ󵵸� �����մϴ�.
    // common_.h �� ���ǵ� ũ�⸦ �������� �ػ󵵸� �����մϴ�.
    // �ػ󵵴� �ּ� �� �ִ� �̹��� �ػ� ���̿��� Ŭ�����˴ϴ�.
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
