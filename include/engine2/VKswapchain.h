#ifndef INCLUDE_VULKAN_SWAPCHAIN_H_
#define INCLUDE_VULKAN_SWAPCHAIN_H_


#include "VKcontext.h"
#include "VKbarrier2.h"

namespace vkengine {

    class VKSwapChain {
    public:
        VKSwapChain(VKcontext& ctx, VkSurfaceKHR surface, VkExtent2D& windowSize, bool vsync = false);
        ~VKSwapChain();

        void initSurface(VkSurfaceKHR surface, cBool vsync);
        void createSwapChain();
        void createImageViews();
        void transitionTo(VkCommandBuffer commandBuffer,cUint32_t imageIndex,VkImageLayout newLayout,VkAccessFlags2 newAccess,VkPipelineStageFlags2 newStage);
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
        VKBarrierHelper& getBarrierHelper(cUint32_t imageIndex);

        VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, cUint32_t& imageIndex) const;
        VkResult queuePresent(VkQueue queue, cUint32_t imageIndex, VkSemaphore waitSemaphore);
        VkImage Image(cUint32_t index);
        VkImageView ImageView(cUint32_t index);
        VkSurfaceKHR& Surface();
        cUint32_t ImageCount() const;
        std::vector<VkImageView>& AllImageViews();
        std::vector<VkImage>& AllImages();

    private:

        std::vector<VkImage> Images{};
        std::vector<VkImageView> ImageViews{};

        VKcontext& ctx;
        VkSurfaceKHR surface;

        VkSwapchainKHR swapChain;
        VkExtent2D windowSize;
        VkFormat ImageFormat;
        VkColorSpaceKHR colorSpace;
        VkPresentModeKHR selectPresentMode;
        cUint32_t imageCount;
        VkSurfaceCapabilitiesKHR capabilities;

        std::vector<VKBarrierHelper> barrierHelpers;

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, cBool vsync);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };

}
#endif // INCLUDE_VULKAN_SWAPCHAIN_H_