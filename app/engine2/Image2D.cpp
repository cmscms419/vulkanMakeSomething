#include "Image2D.h"

#include <algorithm>

#include <ktx.h>
#include <ktxvulkan.h>

#include "log.h"
#include "helper.h"


namespace vkengine
{

    std::string fixPath(const cString &path) // for linux path
    {
        std::string fixed = path;
        std::replace(fixed.begin(), fixed.end(), '\\', '/');
        return fixed;
    }

    Image2D::Image2D(Context &context) : ctx(context)
    {
    }

    Image2D::~Image2D()
    {
        cleanup();
    }

    void Image2D::createImage(
        cUint32_t width,
        cUint32_t height,
        VkFormat format,
        VkSampleCountFlagBits sampleCount,
        VkImageUsageFlags usage,
        VkImageAspectFlags aspectMask,
        cUint32_t mipLevels,
        cUint32_t arrayLayers,
        VkImageCreateFlagBits flags)
    {
        this->cleanup();

        this->width = width;
        this->height = height;
        this->usageFlags = usage;
        this->aspectFlags = aspectMask;

        vkengine::helper::resource::createImage2(
            this->ctx.getDevice()->logicaldevice,
            this->ctx.getDevice()->physicalDevice,
            width,
            height,
            mipLevels,
            sampleCount,
            format,
            VK_IMAGE_TILING_OPTIMAL,
            usage,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->image,
            this->memory,
            arrayLayers,
            flags);

        this->imageView = vkengine::helper::resource::createImageView(
            ctx.getDevice()->logicaldevice,
            this->image, format, aspectMask, mipLevels, arrayLayers);

        this->descriptorCount = 1;
        this->update();
        this->barrierHelper.update(format, mipLevels, arrayLayers);
    }

    void Image2D::createCubeImage(cUint32_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage, VkImageAspectFlags aspectMask, cUint32_t mipLevels, cUint32_t arrayLayers, VkImageCreateFlagBits flags)
    {
        this->cleanup();

        this->width = width;
        this->height = height;
        this->usageFlags = usage;
        this->aspectFlags = aspectMask;

        vkengine::helper::resource::createImage2(
            this->ctx.getDevice()->logicaldevice,
            this->ctx.getDevice()->physicalDevice,
            width,
            height,
            mipLevels,
            sampleCount,
            format,
            VK_IMAGE_TILING_OPTIMAL,
            usage,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->image,
            this->memory,
            arrayLayers,
            flags);

        this->imageView = vkengine::helper::resource::createCubeImageView(
            ctx.getDevice()->logicaldevice,
            this->image, format, aspectMask, mipLevels);

        this->descriptorCount = 1;
        this->update();
        this->barrierHelper.update(format, mipLevels, arrayLayers);
    }

    void Image2D::createTextureFromKTX2(cString filepath, cBool useCubemap)
    {
        filepath = fixPath(filepath);

        size_t extensionPos = filepath.find_last_of('.');
        cString extension = (extensionPos != cString::npos) ? filepath.substr(extensionPos) : "";
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extensionPos == cString::npos || (extension != ".ktx2"))
        {
            Log::exitToLogger("지원하지 않는 이미지 형식입니다: " + extension);
        }

        TextureResourceKTX *resource = nullptr;
        cUint32_t mipLevels = 1;
        VkFormat vkFormat = VK_FORMAT_UNDEFINED;
        ktxTexture *baseTexture = nullptr;
        ktx_uint8_t *ktxTextureData = nullptr;
        ktx_size_t ktxTextureSize = 0;
        cUint32_t layCounter = 0;

        if (extension == ".ktx2")
        {
            resource = new TextureResourceKTX();
            resource->createResource2(filepath);
        }
        else
        {
            Log::exitToLogger("지원하지 않는 이미지 형식입니다: " + extension);
        }

        this->width = resource->texWidth;
        this->height = resource->texHeight;
        
        if (!resource)
        {
            Log::exitToLogger("리소스가 생성되지 않았습니다 ");
        }
        else
        {
            if (!resource->texture2)
            {
                delete resource;
                Log::exitToLogger("KTX2 텍스처 리소스가 유효하지 않습니다.");
            }

            mipLevels = resource->texture2 ? resource->texture2->numLevels : 1;
            vkFormat = ktxTexture2_GetVkFormat(resource->texture2);
            baseTexture = ktxTexture(resource->texture2);
            ktxTextureData = ktxTexture_GetData(baseTexture);
            ktxTextureSize = ktxTexture_GetDataSize(baseTexture);
            layCounter = useCubemap ? 6 : 1;

            if (mipLevels == 0)
            {
                delete resource;
                Log::exitToLogger("KTX2 텍스처의 mipLevels가 유효하지 않습니다.");
            }

            if (vkFormat == VK_FORMAT_UNDEFINED)
            {
                vkFormat = useCubemap ? VK_FORMAT_R16G16B16A16_SFLOAT : VK_FORMAT_R16G16_SFLOAT;
            }

            VkImageCreateFlagBits flags = static_cast<VkImageCreateFlagBits>(useCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0);

            BaseBuffer2 stagingBuffer(this->ctx);
            stagingBuffer.createStagingBuffer(ktxTextureSize, ktxTextureData);

            if (useCubemap)
            {
                this->createCubeImage(
                    this->width, this->height, vkFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layCounter, flags);
            }
            else
            {
                this->createImage(
                    this->width, this->height, vkFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layCounter, flags);
            }

            VkCommandBuffer cmb = this->barrierHelper.beginSingleTimeCommands2(
                ctx.getDevice()->logicaldevice,
                ctx.getDevice()->transferCommandPool,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY);

            this->barrierHelper.transitionImageLayout2(
                cmb, this->image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT);

            vkengine::helper::resource::copyBufferToImageKTX2(
                cmb, stagingBuffer.getBuffer(), this->image,
                resource->texWidth, resource->texHeight,
                mipLevels, baseTexture, useCubemap);

            this->barrierHelper.transitionImageLayout2(
                cmb, this->image,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

            this->barrierHelper.endSingleTimeCommands2(
                ctx.getDevice()->logicaldevice,
                ctx.getDevice()->transferCommandPool,
                ctx.getDevice()->transferVKQueue,
                cmb);
            delete resource;
        }
    }

    void Image2D::createTextureFromKTX1(cString filepath, cBool useCubemap)
    {
        filepath = fixPath(filepath);

        size_t extensionPos = filepath.find_last_of('.');
        cString extension = (extensionPos != cString::npos) ? filepath.substr(extensionPos) : "";
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extensionPos == cString::npos || (extension != ".ktx"))
        {
            Log::exitToLogger("지원하지 않는 이미지 형식입니다: " + extension);
        }

        TextureResourceKTX *resource = new TextureResourceKTX();
        resource->createResource(filepath);

        if (!resource->texture)
        {
            delete resource;
            Log::exitToLogger("KTX 텍스처 리소스가 유효하지 않습니다.");
        }

        ktxTexture *baseTexture = resource->texture;
        cUint32_t mipLevels = baseTexture->numLevels;
        cUint32_t layCounter = useCubemap ? 6 : 1;
        VkFormat vkFormat = ktxTexture_GetVkFormat(baseTexture);
        ktx_uint8_t *ktxTextureData = ktxTexture_GetData(baseTexture);
        ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(baseTexture);

        if (mipLevels == 0)
        {
            delete resource;
            Log::exitToLogger("KTX 텍스처의 mipLevels가 유효하지 않습니다.");
        }

        if (useCubemap && baseTexture->numFaces != 6)
        {
            delete resource;
            Log::exitToLogger("큐브맵을 요청했지만 KTX 텍스처의 face 수가 6이 아닙니다.");
        }

        if (vkFormat == VK_FORMAT_UNDEFINED)
        {
            delete resource;
            Log::exitToLogger("KTX 텍스처의 VkFormat을 확인할 수 없습니다.");
        }

        this->width = resource->texWidth;
        this->height = resource->texHeight;

        VkImageCreateFlagBits flags = static_cast<VkImageCreateFlagBits>(useCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0);

        BaseBuffer2 stagingBuffer(this->ctx);
        stagingBuffer.createStagingBuffer(ktxTextureSize, ktxTextureData);

        if (useCubemap)
        {
            this->createCubeImage(
                this->width, this->height, vkFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layCounter, flags);
        }
        else
        {
            this->createImage(
                this->width, this->height, vkFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layCounter, flags);
        }

        VkCommandBuffer cmb = this->barrierHelper.beginSingleTimeCommands2(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->transferCommandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        this->barrierHelper.transitionImageLayout2(
            cmb, this->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT);

        vkengine::helper::resource::copyBufferToImageKTX2(
            cmb, stagingBuffer.getBuffer(), this->image,
            this->width, this->height,
            mipLevels, baseTexture, useCubemap);

        this->barrierHelper.transitionImageLayout2(
            cmb, this->image,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_2_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

        this->barrierHelper.endSingleTimeCommands2(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->transferCommandPool,
            ctx.getDevice()->transferVKQueue,
            cmb);

        delete resource;
    }

    void Image2D::createTextureFromPNG(cString filepath, cBool useCubemap, cBool sRGB)
    {

        TextureResourceBase *resource = nullptr;

        resource = new TextureResourcePNG();
        resource->createResource(filepath);

        if (!resource->data)
        {
            Log::exitToLogger("pixelData is nullptr");
        }

        VkFormat imageFormat{};

        switch (resource->texChannels)
        {
        case 4:
            imageFormat = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
            break;
        default:
            Log::exitToLogger("지원하지 않는 이미지 채널 수입니다: " + std::to_string(resource->texChannels));
            break;
        }

        this->width = resource->texWidth;
        this->height = resource->texHeight;

        VkDeviceSize imageSize = width * height * resource->texChannels * sizeof(cUChar);

        BaseBuffer2 stagingBuffer(this->ctx);
        stagingBuffer.createStagingBuffer(imageSize, resource->data);

        this->createImage(
            width, height, imageFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);

        VkCommandBuffer cmb = this->barrierHelper.beginSingleTimeCommands2(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->transferCommandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        this->barrierHelper.transitionImageLayout2(
            cmb, this->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT);

        vkengine::helper::resource::copyBufferToImage3(
            cmb, stagingBuffer.getBuffer(), this->image, width, height);

        this->barrierHelper.transitionImageLayout2(
            cmb, this->image,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_2_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

        this->barrierHelper.endSingleTimeCommands2(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->transferCommandPool,
            ctx.getDevice()->transferVKQueue,
            cmb);
    }

    void Image2D::createTextureFromImage(cString file, cBool useCubemap, cBool sRGB)
    {
        // 추후 ktx 등 여러 파일 포맷을 로드할 수 있게 만들어야 함
        size_t extensionPos = file.find_last_of('.');
        cString extension = (extensionPos != cString::npos) ? file.substr(extensionPos) : "";
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extensionPos == cString::npos ||
            (extension != ".png" && extension != ".jpg" && extension != ".jpeg" && extension != ".ktx"))
        {
            Log::exitToLogger("지원하지 않는 이미지 형식입니다: " + extension);
        }

        if (useCubemap)
        {
            Log::exitToLogger("PNG, JPG는 큐브맵을 제공하지 않습니다.");
        }

        if (extension == ".png")
        {
            this->createTextureFromPNG(file, useCubemap, sRGB);
        }
        else if (extension == ".ktx")
        {
            this->createTextureFromKTX1(file, useCubemap);
        }
        else if (extension == ".ktx2")
        {
            this->createTextureFromKTX2(file, useCubemap);
        }
        else
        {
            vkengine::Log::exitToLogger("Failed to create texture: resource is null.");
        }
    }

    void Image2D::createTextureFromPixelData(cUChar *pixelData, cUint32_t width, cUint32_t height, cUint32_t channels, cBool sRGB)
    {
        if (!pixelData)
        {
            Log::exitToLogger("pixelData is nullptr");
        }

        VkFormat imageFormat{};

        switch (channels)
        {
        case 4:
            imageFormat = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
            break;
        default:
            Log::exitToLogger("지원하지 않는 이미지 채널 수입니다: " + std::to_string(channels));
            break;
        }

        VkDeviceSize imageSize = width * height * channels * sizeof(cUChar);

        BaseBuffer2 stagingBuffer(this->ctx);
        stagingBuffer.createStagingBuffer(imageSize, pixelData);

        this->createImage(
            width, height, imageFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);

        VkCommandBuffer cmb = this->barrierHelper.beginSingleTimeCommands2(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->transferCommandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        this->barrierHelper.transitionImageLayout2(
            cmb, this->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT);

        vkengine::helper::resource::copyBufferToImage3(
            cmb, stagingBuffer.getBuffer(), this->image, width, height);

        this->barrierHelper.transitionImageLayout2(
            cmb, this->image,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_2_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

        this->barrierHelper.endSingleTimeCommands2(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->transferCommandPool,
            ctx.getDevice()->transferVKQueue,
            cmb);
    }

    void Image2D::createGeneralStorage(cUint16_t width, cUint32_t height, VkFormat format)
    {
        VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        this->createImage(static_cast<cUint32_t>(width), static_cast<cUint32_t>(height),
                          format, VK_SAMPLE_COUNT_1_BIT, usage,
                          VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, static_cast<VkImageCreateFlagBits>(0));
    }

    void Image2D::createShadowMap(cUint16_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount)
    {
        VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        this->createImage(width, height, format, sampleCount, usage,
                          VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, static_cast<VkImageCreateFlagBits>(0));
    }

    void Image2D::createDepthStencil(cUint32_t width, cUint32_t height, cBool onlyDepth)
    {
        this->usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        this->width = width;
        this->height = height;
        this->aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

        VkFormat format = ctx.getDepthStencil()->depthFormat;

        if (!onlyDepth)
        {
            this->aspectFlags |= ((format >= VK_FORMAT_D16_UNORM_S8_UINT) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
        }

        helper::resource::createImage(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->physicalDevice,
            this->width,
            this->height,
            1,
            VK_SAMPLE_COUNT_1_BIT, //  Always use 1x samples (no MSAA)
            format,
            VK_IMAGE_TILING_OPTIMAL,
            this->usageFlags,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            image,
            memory);

        this->imageView = vkengine::helper::resource::createImageView(
            ctx.getDevice()->logicaldevice,
            this->image,
            format,
            this->aspectFlags,
            1,
            1);

        this->descriptorCount = 1;
        this->update();
        this->barrierHelper.update(format, 1, 1);
    }

    void Image2D::cleanup()
    {
        if (imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(ctx.getDevice()->logicaldevice, imageView, nullptr);
            imageView = VK_NULL_HANDLE;
        }

        if (image != VK_NULL_HANDLE)
        {
            vkDestroyImage(ctx.getDevice()->logicaldevice, image, nullptr);
            image = VK_NULL_HANDLE;
        }

        if (memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(ctx.getDevice()->logicaldevice, memory, nullptr);
            memory = VK_NULL_HANDLE;
        }


        this->barrierHelper.getCurrentLayout() = VK_IMAGE_LAYOUT_UNDEFINED;
        this->barrierHelper.getCurrentAccess() = VK_ACCESS_2_NONE;
        this->barrierHelper.getCurrentStage() = VK_PIPELINE_STAGE_2_NONE;
    }

}
