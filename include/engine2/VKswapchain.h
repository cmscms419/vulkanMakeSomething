#ifndef INCLUDE_VULKAN_SWAPCHAIN_H_
#define INCLUDE_VULKAN_SWAPCHAIN_H_


#include "VKcontext.h"
#include "VKbarrier2.h"
#include "helper.h"
#include "log.h"

namespace vkengine {

    class VKSwapChain {
    public:
        VKSwapChain(VKcontext& ctx, VkSurfaceKHR surface, VkExtent2D& windowSize, bool vsync = false);
        ~VKSwapChain() {
            cleanup();
        }

        void initSurface(VkSurfaceKHR surface, cBool vsync);
        void createSwapChain();
        void createImageViews();

        VkExtent2D getWindowSize() const { return this->windowSize; }
        VkFormat getSwapChainImageFormat() const { return this->ImageFormat; }
        VkSwapchainKHR getSwapChain() const { return this->swapChain; }

        VkImage& getSwapChainImage(cUint32_t imageindex) {
            if (imageindex >= this->Images.size()) {
                vkengine::Log::EXIT_TO_LOGGER("SwapChain image index out of range");
            }

            if (this->Images.empty()) {
                vkengine::Log::EXIT_TO_LOGGER("SwapChain images is empty");
            }

            if (this->Images[imageindex] == VK_NULL_HANDLE) {
                vkengine::Log::EXIT_TO_LOGGER("SwapChain image is null handle");
            }

            return this->Images[imageindex];
        }

        std::vector<VkImage>& getSwapChainImages() { return this->Images; }

        VkImageView& getSwapChainImageView(cUint32_t imageindex) {
            if (imageindex >= this->ImageViews.size()) {
                vkengine::Log::EXIT_TO_LOGGER("SwapChain image view index out of range");
            }
            if (this->ImageViews.empty()) {
                vkengine::Log::EXIT_TO_LOGGER("SwapChain image views is empty");
            }
            if (this->ImageViews[imageindex] == VK_NULL_HANDLE) {
                vkengine::Log::EXIT_TO_LOGGER("SwapChain image view is null handle");
            }
            return this->ImageViews[imageindex];
        }

        std::vector<VkImageView>& getSwapChainImageViews() { return this->ImageViews; }

        cUint32_t getSwapChainImageCount() const { return static_cast<cUint32_t>(this->Images.size()); }

        VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, cUint32_t& imageIndex) const
        {
            return vkAcquireNextImageKHR(this->ctx.getDevice()->logicaldevice, this->swapChain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, &imageIndex);
        }

        VkResult queuePresent(VkQueue queue, cUint32_t imageIndex, VkSemaphore waitSemaphore);

        void cleanup();

        VkImage Image(cUint32_t index)
        {
            VkImage result = VK_NULL_HANDLE;

            if (!this->Images.empty() || index < this->Images.size())
                result = Images[index];

            return result;
        }

        VkImageView ImageView(cUint32_t index)
        {
            VkImageView result = VK_NULL_HANDLE;

            if (!this->ImageViews.empty() || index < this->ImageViews.size())
                result = ImageViews[index];

            return result;
        }

        VkSurfaceKHR& Surface() {
            return this->surface;
        }
    
        cUint32_t ImageCount() const {
            return this->imageCount;
        }

        std::vector<VkImageView>& AllImageViews() {
            return this->ImageViews;
        }

        std::vector<VkImage>& AllImages() {
            return this->Images;
        }

        VKBarrierHelper& getBarrierHelper(cUint32_t imageIndex) {
            return barrierHelpers[imageIndex];
        }

    private:

        std::vector<VkImage> Images{};
        std::vector<VkImageView> ImageViews{};

        VKcontext& ctx;
        VkSurfaceKHR surface{ VK_NULL_HANDLE };

        VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
        VkExtent2D windowSize{ 0, 0 };
        VkFormat ImageFormat{ VK_FORMAT_UNDEFINED };
        VkColorSpaceKHR colorSpace{ VK_COLOR_SPACE_MAX_ENUM_KHR };
        VkPresentModeKHR selectPresentMode{ VK_PRESENT_MODE_MAX_ENUM_KHR };
        cUint32_t imageCount{ 0 };
        VkSurfaceCapabilitiesKHR capabilities = {};

        std::vector<VKBarrierHelper> barrierHelpers;

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, cBool vsync);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };

}
#endif // INCLUDE_VULKAN_SWAPCHAIN_H_