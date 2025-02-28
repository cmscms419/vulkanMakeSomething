#ifndef INCLUDE_VULKANSWAPCHAIN_H_
#define INCLUDE_VULKANSWAPCHAIN_H_

#include "../_common.h"
#include "../struct.h"

namespace vkengine {

    class VKSwapChain {
    public:
        VKSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkInstance* Instance);
        ~VKSwapChain() = default;
        
        void createSwapChain(QueueFamilyIndices* VKqueueFamilyIndices);
        void createImageViews();
        void cleanupSwapChain();

        const VkExtent2D getSwapChainExtent() { return this->VKswapChainExtent; }
        const VkFormat getSwapChainImageFormat() { return this->VKswapChainImageFormat; }
        const VkSwapchainKHR getSwapChain() { return this->VKswapChain; }

        std::vector<VkImage> getSwapChainImages() { return this->VKswapChainImages; }
        std::vector<VkImageView> getSwapChainImageViews() { return this->VKswapChainImageViews; }
        VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex);
        uint32_t getSwapChainImageCount() { return static_cast<uint32_t>(this->VKswapChainImages.size()); }
    private:

        VkSwapchainKHR VKswapChain{ VK_NULL_HANDLE };
        VkExtent2D VKswapChainExtent{ 0, 0 };
        VkFormat VKswapChainImageFormat{ VK_FORMAT_UNDEFINED };

        std::vector<VkImage> VKswapChainImages{};
        std::vector<VkImageView> VKswapChainImageViews{};

        VkPhysicalDevice VKphysicalDevice{ VK_NULL_HANDLE };
        VkDevice VKdevice{ VK_NULL_HANDLE };
        VkSurfaceKHR VKsurface{ VK_NULL_HANDLE };
        VkInstance VKInstance{ VK_NULL_HANDLE };

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };

}
#endif // INCLUDE_VULKANSWAPCHAIN_H_