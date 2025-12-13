#ifndef INCLUDE_VK_IMAGE_2D_H_
#define INCLUDE_VK_IMAGE_2D_H_

#include "VKContext.h"
#include "VKbuffer2.h"
#include "VKResourceBindingData.h"

namespace vkengine {
    class VKImage2D
    {
    public:
        VKImage2D(VKcontext& context);
        VKImage2D(const VKImage2D&) = delete;
        VKImage2D(VKImage2D&& other) noexcept;
        VKImage2D& operator=(const VKImage2D&) = delete;
        VKImage2D& operator=(VKImage2D&&) = delete;
        ~VKImage2D();

        VKBarrierHelper& getBarrierHelper() { return resourceBinding.getBarrierHelper(); }
        VKResourceBinding& getResourceBinding() { return resourceBinding; }

        void createImage(cUint32_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage,
            VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers,
            VkImageCreateFlagBits flags);

        void createCubeImage(cUint32_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage,
            VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers,
            VkImageCreateFlagBits flags);

        void createTextureFromKtx2(cString filepath, cBool usCubemap);
        void createTextureFromImage(cString filepath, cBool usCubemap, cBool sRGB);
        void createTextureFromPixelData(cUChar* pixelData, cUint32_t width, cUint32_t height, cUint32_t channels, cBool sRGB);

        void cleanup();

        // 이미지 레이아웃 전환
        // 정해진 템플릿 없음
        void transitionTo(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkAccessFlags2 newAccess, VkPipelineStageFlags2 newStage)
        {
            resourceBinding.getBarrierHelper().transitionImageLayout2(
                commandBuffer,
                newLayout,
                newAccess,
                newStage
            );
        }

        // 이미지 ColorAttachment 변환
        void transitionToColorAttachment(VkCommandBuffer commandBuffer)
        {
            transitionTo(
                commandBuffer,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
            );
        }

        void transitionToTransferSrc(VkCommandBuffer commandBuffer)
        {
            transitionTo(
                commandBuffer,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_ACCESS_2_TRANSFER_READ_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT
            );
        }

        VkImage getImage() const { return image; }
        VkImageView getImageView() const { return imageView; }
        VkFormat getImageFormat() const { return imageFormat; }
        cUint32_t getWidth() const { return width; }
        void setSampler(VkSampler sampler) {
            this->resourceBinding.setSampler(sampler);
        }

    private:
        VKcontext& ctx;

        VkImage image{ VK_NULL_HANDLE };
        VkDeviceMemory imageMemory{ VK_NULL_HANDLE };
        VkImageView imageView{ VK_NULL_HANDLE };
        VkFormat imageFormat{ VK_FORMAT_UNDEFINED };

        cUint32_t width{ 0 };
        cUint32_t height{ 0 };

        VkImageUsageFlags usageFlags{ 0 };
        VkImageAspectFlags aspectFlags{ 0 };

        VKResourceBinding resourceBinding;
    };
}

#endif // !INCLUDE_VK_IMAGE_H_
