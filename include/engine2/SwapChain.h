#ifndef INCLUDE_VULKAN_SWAPCHAIN_H_
#define INCLUDE_VULKAN_SWAPCHAIN_H_


#include "Context.h"
#include "BarrierHelper.h"

namespace vkengine {

    class SwapChain {
    public:
        SwapChain(Context& ctx, VkSurfaceKHR surface, VkExtent2D& windowSize, bool vsync = false);
        ~SwapChain();

        void initSurface(VkSurfaceKHR surface, cBool vsync);
        void createSwapChain();
        void createImageViews();
        void transitionTo(VkCommandBuffer commandBuffer,cUint32_t imageIndex);
        void cleanup();
        void cleanupWithoutSurface();

        VkExtent2D getWindowSize() const;
        VkFormat getSwapChainImageFormat() const;
        VkSwapchainKHR& getSwapChain();
        std::vector<VkImage>& getSwapChainImages();
        std::vector<VkImageView>& getSwapChainImageViews();
        VkImageView& getSwapChainImageView(cUint32_t imageindex);
        VkImage& getSwapChainImage(cUint32_t imageindex);
        cUint32_t getSwapChainImageCount() const;
        BarrierHelper& getBarrierHelper(cUint32_t imageIndex);

        VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, cUint32_t& imageIndex) const;
        VkResult queuePresent(VkQueue queue, cUint32_t imageIndex, VkSemaphore waitSemaphore);
        VkSurfaceKHR& getSurface();

    private:

        std::vector<VkImage> Images{};
        std::vector<VkImageView> ImageViews{};

        Context& ctx;
        VkSurfaceKHR surface;

        VkSwapchainKHR swapChain;
        VkExtent2D windowSize;
        VkFormat ImageFormat;
        VkColorSpaceKHR colorSpace;
        VkPresentModeKHR selectPresentMode;
        cUint32_t imageCount;
        VkSurfaceCapabilitiesKHR capabilities;

        std::vector<BarrierHelper> barrierHelpers;

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, cBool vsync);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };

}

#endif // INCLUDE_VULKAN_SWAPCHAIN_H_
