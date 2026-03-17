#ifndef INCLUDE_VK_IMAGE_2D_H_
#define INCLUDE_VK_IMAGE_2D_H_

#include "VKContext.h"
#include "VKbuffer2.h"
#include "VKShaderResource.h"

namespace vkengine
{
    class VKImage2D : public VKShaderResource
    {
    public:
        VKImage2D(VKcontext &context);
        VKImage2D(VKImage2D &&other) = delete;
        VKImage2D(const VKImage2D &) = delete;
        VKImage2D &operator=(const VKImage2D &) = delete;
        VKImage2D &operator=(VKImage2D &&) = delete;
        ~VKImage2D();

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
        void createDepthStencil(cUint32_t width, cUint32_t height, VkSampleCountFlagBits msaaSamples);
        void updateResourceBindingAfterTransition();
        void cleanup() override;

        void transitionTo(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkAccessFlags2 newAccess, VkPipelineStageFlags2 newStage);
        void transitionToColorAttachment(VkCommandBuffer commandBuffer);
        void transitionToDepthStencilAttachment(VkCommandBuffer commandBuffer);
        void transitionToTransferDst(VkCommandBuffer commandBuffer);
        void transitionToTransferSrc(VkCommandBuffer commandBuffer);
        void transitionToShaderReadOnly(VkCommandBuffer commandBuffer);
        void transitionToShaderReadWrite(VkCommandBuffer commandBuffer);
        void transitionToPresent(VkCommandBuffer commandBuffer);

        virtual void updateBinding(VkDescriptorSetLayoutBinding &binding) override;
        virtual void updateWrite(VkWriteDescriptorSet &write) override;

        VkImage getImage()         { return this->image; }
        VkImageView getImageView() { return this->imageView; }
        VkImageView getSamplerView() { return this->samplerView; }
        VkFormat getImageFormat()  { return this->imageFormat; }
        cUint32_t getHeight()      { return this->height; }
        cUint32_t getWidth()       { return this->width; }

    private:
        VKcontext &ctx;
        VkFormat imageFormat{VK_FORMAT_UNDEFINED};
        VkImageView samplerView{VK_NULL_HANDLE};

        cUint32_t width{0};
        cUint32_t height{0};

        VkImageUsageFlags usageFlags{0};
        VkImageAspectFlags aspectFlags{0};
    };
}

#endif // !INCLUDE_VK_IMAGE_H_
