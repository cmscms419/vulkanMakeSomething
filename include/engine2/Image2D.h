#ifndef INCLUDE_VK_IMAGE_2D_H_
#define INCLUDE_VK_IMAGE_2D_H_

#include "Context.h"
#include "BaseBuffer2.h"
#include "ShaderResource.h"

namespace vkengine
{
    class Image2D : public ImageShaderResource
    {
    public:
        Image2D(Context &context);
        Image2D(Image2D &&other) = delete;
        Image2D(const Image2D &) = delete;
        Image2D &operator=(const Image2D &) = delete;
        Image2D &operator=(Image2D &&) = delete;
        ~Image2D();

        void createImage(cUint32_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage,
                         VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers, VkImageCreateFlagBits flags);

        void createCubeImage(cUint32_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage,
                             VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers, VkImageCreateFlagBits flags);

        void createTextureFromKTX2(cString filepath, cBool useCubemap);
        void createTextureFromKTX1(cString filepath, cBool useCubemap);
        void createTextureFromPNG(cString filepath, cBool useCubemap, cBool sRGB);
        void createTextureFromImage(cString filepath, cBool useCubemap, cBool sRGB);
        void createTextureFromPixelData(cUChar *pixelData, cUint32_t width, cUint32_t height, cUint32_t channels, cBool sRGB);
        void createGeneralStorage(cUint16_t width, cUint32_t height, VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT);
        void createShadowMap(cUint16_t width, cUint32_t height, VkFormat format = VK_FORMAT_D16_UNORM, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
        void createDepthStencil(cUint32_t width, cUint32_t height, cBool onlyDepth = false);
        void cleanup() override;

        VkImageView getImageView() { return this->imageView; }
        cUint32_t getHeight() { return this->height; }
        cUint32_t getWidth() { return this->width; }

    private:
        Context &ctx;

        cUint32_t width{0};
        cUint32_t height{0};

        VkImageUsageFlags usageFlags{0};
        VkImageAspectFlags aspectFlags{0};
    };
}

#endif // !INCLUDE_VK_IMAGE_H_
