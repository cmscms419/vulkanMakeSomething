#ifndef INCLUDE_VK_IMAGE_2D_H_
#define INCLUDE_VK_IMAGE_2D_H_

#include "VKContext.h"
#include "VKShaderResource.h"
#include "VKbuffer2.h"
#include "VKResourceBindingData.h"

namespace vkengine
{
    class VKImage2D : public VKShaderResource
    {
    public:
        VKImage2D(VKcontext &context);
        VKImage2D(const VKImage2D &) = delete;
        VKImage2D(VKImage2D &&other) noexcept;
        VKImage2D &operator=(const VKImage2D &) = delete;
        VKImage2D &operator=(VKImage2D &&) = delete;
        ~VKImage2D();

        VKBarrierHelper &getBarrierHelper() { return resourceBinding.getBarrierHelper(); }

        void createImage(cUint32_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage,
                         VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers, VkImageCreateFlagBits flags);

        void createCubeImage(cUint32_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage,
                             VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers, VkImageCreateFlagBits flags);

        void createTextureFromKtx2(cString filepath, cBool usCubemap);
        void createTextureFromImage(cString filepath, cBool usCubemap, cBool sRGB);
        void createTextureFromPixelData(cUChar *pixelData, cUint32_t width, cUint32_t height, cUint32_t channels, cBool sRGB);
        void createMsaaColorBuffer(cUint16_t width, cUint16_t height, VkSampleCountFlagBits sampleCount);
        void createGeneralStorage(cUint16_t width, cUint32_t height);
        void createShadowMap(cUint16_t width, cUint32_t height, VkFormat format = VK_FORMAT_D16_UNORM, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
        void updateResourceBindingAfterTransition();
        void cleanup();

        // 이미지 레이아웃 전환
        // 정해진 템플릿 없음
        void transitionTo(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkAccessFlags2 newAccess, VkPipelineStageFlags2 newStage);

        // 이미지 ColorAttachment 변환
        void transitionToColorAttachment(VkCommandBuffer commandBuffer);

        void transitionToTransferSrc(VkCommandBuffer commandBuffer);

        VkImage getImage();
        VkImageView getImageView();
        VkFormat getImageFormat();
        cUint32_t getHeight();
        cUint32_t getWidth();
        
        VKResourceBinding &getResourceBinding() override
        {
            return this->resourceBinding;
        }

        const VKResourceBinding &getResourceBinding() const override
        {
            return this->resourceBinding;
        }

        void setSampler(VkSampler sampler)
        {
            this->resourceBinding.setSampler(sampler);
        }

        void updateBinding(VkDescriptorSetLayoutBinding &binding) override
        {
            binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding.descriptorCount = 1;
            binding.stageFlags = 0;
        }

        void updateWrite(VkWriteDescriptorSet &write) override
        {
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount = 1;
            write.pImageInfo = &resourceBinding.imageInfo;
        }

    private:
        VKcontext &ctx;

        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
        VkFormat imageFormat;

        cUint32_t width;
        cUint32_t height;

        VkImageUsageFlags usageFlags;
        VkImageAspectFlags aspectFlags;

        VKResourceBinding resourceBinding;
    };
}

#endif // !INCLUDE_VK_IMAGE_H_
