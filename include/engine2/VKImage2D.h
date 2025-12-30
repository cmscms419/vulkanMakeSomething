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
        void createMsaaColorBuffer(cUint16_t width, cUint16_t height, VkSampleCountFlagBits sampleCount);
        void createGeneralStorage(cUint16_t width, cUint32_t height);
        void updateResourceBindingAfterTransition()
        {
            VkImageLayout currentLayout = resourceBinding.barrierHelper.Currentlayout();

            if (currentLayout == VK_IMAGE_LAYOUT_GENERAL) {
                // General layout is used for storage images
                resourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                resourceBinding.imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            }
            else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                // Shader read-only layout is used for sampled images
                if (resourceBinding.sampler != VK_NULL_HANDLE) {
                    resourceBinding.descriptorType= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                }
                else {
                    resourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                }
                resourceBinding.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            else if (currentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                currentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                // Attachment layouts are typically used for input attachments when used in descriptors
                resourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                resourceBinding.imageInfo.imageLayout = currentLayout;
            }
            else {
                // For other layouts, default to storage image with general layout capability
                resourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                resourceBinding.imageInfo.imageLayout = currentLayout;
            }

            // Update the image info
            resourceBinding.imageInfo.imageView = imageView;
            resourceBinding.imageInfo.sampler = resourceBinding.sampler;
        }
        void cleanup();

        // 이미지 레이아웃 전환
        // 정해진 템플릿 없음
        void transitionTo(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkAccessFlags2 newAccess, VkPipelineStageFlags2 newStage)
        {
            resourceBinding.getBarrierHelper().transitionImageLayout2(
                commandBuffer,
                this->image,
                newLayout,
                newAccess,
                newStage
            );
            updateResourceBindingAfterTransition();
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
            updateResourceBindingAfterTransition();
        }

        void transitionToTransferSrc(VkCommandBuffer commandBuffer)
        {
            transitionTo(
                commandBuffer,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_ACCESS_2_TRANSFER_READ_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT
            );
            updateResourceBindingAfterTransition();
        }

        VkImage getImage() { return image; }
        VkImageView getImageView() { return imageView; }
        VkFormat getImageFormat() { return imageFormat; }
        cUint32_t getWidth() { return width; }
        VKResourceBinding& ResourceBinding()
        {
            return this->resourceBinding;
        }
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
