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

        // ���� ü�� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = this->surface;

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
