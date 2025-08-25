#ifndef INCLUDE_VULKANSWAPCHAIN_H_
#define INCLUDE_VULKANSWAPCHAIN_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "helper.h"

namespace vkengine {

    class VKSwapChain {
    public:
        VKSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkInstance* Instance);
        ~VKSwapChain() = default;

        void createSwapChain(QueueFamilyIndices* VKqueueFamilyIndices);
        void createImageViews();
        void cleanupSwapChain();

        VkExtent2D getSwapChainExtent() const { return this->Extent; }
        VkFormat getSwapChainImageFormat() const { return this->ImageFormat; }
        VkSwapchainKHR getSwapChain() const { return this->swapChain; }

        std::vector<VkImage> getSwapChainImages() { return this->Images; }
        std::vector<VkImageView> getSwapChainImageViews() { return this->ImageViews; }
        uint32_t getSwapChainImageCount() const { return static_cast<uint32_t>(this->Images.size()); }

        VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex) const
        {
            return vkAcquireNextImageKHR(this->logicaldevice, this->swapChain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, &imageIndex);
        }

    private:

        VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
        VkExtent2D Extent{ 0, 0 };
        VkFormat ImageFormat{ VK_FORMAT_UNDEFINED };

        std::vector<VkImage> Images{};
        std::vector<VkImageView> ImageViews{};

        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
        VkDevice logicaldevice{ VK_NULL_HANDLE };
        VkSurfaceKHR surface{ VK_NULL_HANDLE };
        VkInstance Instance{ VK_NULL_HANDLE };

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };

}
#endif // INCLUDE_VULKANSWAPCHAIN_H_