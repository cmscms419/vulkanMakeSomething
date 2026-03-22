#include "VKImage2D.h"

#include <algorithm>

#include <ktx.h>
#include <ktxvulkan.h>

#include "log.h"
#include "helper.h"

using namespace vkengine::Log;

namespace vkengine
{

    std::string fixPath(const cString &path) // for linux path
    {
        std::string fixed = path;
        std::replace(fixed.begin(), fixed.end(), '\\', '/');
        return fixed;
    }

    VKImage2D::VKImage2D(VKcontext &context) : ctx(context)
    {
    }

    VKImage2D::~VKImage2D()
    {
        cleanup();
    }

    void VKImage2D::createImage(
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

        this->imageFormat = format;
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
        this->barrierHelper.update(this->imageFormat, mipLevels, arrayLayers);
    }

    void VKImage2D::createCubeImage(cUint32_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage, VkImageAspectFlags aspectMask, cUint32_t mipLevels, cUint32_t arrayLayers, VkImageCreateFlagBits flags)
    {
        this->cleanup();

        this->imageFormat = format;
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
        this->barrierHelper.update(this->imageFormat, mipLevels, arrayLayers);
    }

    void VKImage2D::createTextureFromKtx2(cString filepath, cBool usCubemap)
    {
        filepath = fixPath(filepath);

        size_t extensionPos = filepath.find_last_of('.');
        cString extension = (extensionPos != cString::npos) ? filepath.substr(extensionPos) : "";
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extensionPos == cString::npos || (extension != ".ktx2"))
        {
            EXIT_TO_LOGGER("지원하지 않는 이미지 형식입니다: " + extension);
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
            EXIT_TO_LOGGER("지원하지 않는 이미지 형식입니다: " + extension);
        }

        if (!resource)
        {
            EXIT_TO_LOGGER("리소스가 생성되지 않았습니다 ");
        }
        else
        {
            if (!resource->texture2)
            {
                delete resource;
                EXIT_TO_LOGGER("KTX2 텍스처 리소스가 유효하지 않습니다.");
            }

            mipLevels = resource->texture2 ? resource->texture2->numLevels : 1;
            vkFormat = ktxTexture2_GetVkFormat(resource->texture2);
            baseTexture = ktxTexture(resource->texture2);
            ktxTextureData = ktxTexture_GetData(baseTexture);
            ktxTextureSize = ktxTexture_GetDataSize(baseTexture);
            layCounter = usCubemap ? 6 : 1;

            if (mipLevels == 0)
            {
                delete resource;
                EXIT_TO_LOGGER("KTX2 텍스처의 mipLevels가 유효하지 않습니다.");
            }

            if (vkFormat == VK_FORMAT_UNDEFINED)
            {
                vkFormat = usCubemap ? VK_FORMAT_R16G16B16A16_SFLOAT : VK_FORMAT_R16G16_SFLOAT;
            }

            VkImageCreateFlagBits flags = static_cast<VkImageCreateFlagBits>(usCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0);

            VKBaseBuffer2 stagingBuffer(this->ctx);
            stagingBuffer.createStagingBuffer(ktxTextureSize, ktxTextureData);

            if (usCubemap)
            {
                this->createCubeImage(
                    resource->texWidth, resource->texHeight, vkFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, layCounter, flags);
            }
            else
            {
                this->createImage(
                    resource->texWidth, resource->texHeight, vkFormat,
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
                cmb, stagingBuffer.Buffer(), this->image,
                resource->texWidth, resource->texHeight,
                mipLevels, baseTexture, usCubemap);

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

    void VKImage2D::createTextureFromImage(cString file, cBool usCubemap, cBool sRGB)
    {
        size_t extensionPos = file.find_last_of('.');
        cString extension = (extensionPos != cString::npos) ? file.substr(extensionPos) : "";
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extensionPos == cString::npos ||
            (extension != ".png" && extension != ".jpg" && extension != ".jpeg"))
        {
            EXIT_TO_LOGGER("지원하지 않는 이미지 형식입니다: " + extension);
        }

        if (usCubemap)
        {
            EXIT_TO_LOGGER("PNG, JPG는 큐브맵을 제공하지 않습니다.");
        }

        TextureResourceBase *resource = nullptr;

        if (extension == ".png")
        {
            resource = new TextureResourcePNG();
            resource->createResource(file);
        }
        else
        {
            EXIT_TO_LOGGER("지원하지 않는 이미지 형식입니다: " + extension);
        }

        if (resource != nullptr)
        {
            this->createTextureFromPixelData(resource->data, resource->texWidth, resource->texHeight, resource->texChannels, sRGB);
        }
        else
        {
            vkengine::Log::EXIT_TO_LOGGER("Failed to create texture: resource is null.");
        }
    }

    void VKImage2D::createTextureFromPixelData(cUChar *pixelData, cUint32_t width, cUint32_t height, cUint32_t channels, cBool sRGB)
    {
        if (!pixelData)
        {
            EXIT_TO_LOGGER("pixelData is nullptr");
        }

        VkFormat imageFormat{};

        switch (channels)
        {
        case 4:
            imageFormat = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
            break;
        default:
            EXIT_TO_LOGGER("지원하지 않는 이미지 채널 수입니다: " + std::to_string(channels));
            break;
        }

        VkDeviceSize imageSize = width * height * channels * sizeof(cUChar);

        VKBaseBuffer2 stagingBuffer(this->ctx);
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
            cmb, stagingBuffer.Buffer(), this->image, width, height);

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

    void VKImage2D::createMsaaColorBuffer(cUint16_t width, cUint16_t height, VkSampleCountFlagBits sampleCount)
    {
        this->createImage(static_cast<cUint32_t>(width), static_cast<cUint32_t>(height),
                          VK_FORMAT_R16G16B16A16_SFLOAT, sampleCount, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                          VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, static_cast<VkImageCreateFlagBits>(0));
    }

    void VKImage2D::createGeneralStorage(cUint16_t width, cUint32_t height)
    {
        VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        this->createImage(static_cast<cUint32_t>(width), static_cast<cUint32_t>(height),
                    VK_FORMAT_R16G16B16A16_SFLOAT, VK_SAMPLE_COUNT_1_BIT, usage,
                    VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, static_cast<VkImageCreateFlagBits>(0));
    }

    void VKImage2D::createShadowMap(cUint16_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount)
    {
        VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        this->createImage(width, height, format, sampleCount, usage,
                    VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, static_cast<VkImageCreateFlagBits>(0));
    }

    void VKImage2D::createDepthStencil(cUint32_t width, cUint32_t height, VkSampleCountFlagBits msaaSamples, cBool onlyDepth)
    {
        this->usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        this->imageFormat = ctx.getDepthStencil()->depthFormat;
        this->width = width;
        this->height = height;
        this->aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
        
        if (!onlyDepth)
        {
            this->aspectFlags |= ((this->imageFormat >= VK_FORMAT_D16_UNORM_S8_UINT) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
        }

        helper::resource::createImage(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->physicalDevice,
            this->width,
            this->height,
            1,
            msaaSamples,
            this->imageFormat,
            VK_IMAGE_TILING_OPTIMAL,
            this->usageFlags,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            image,
            memory);

        this->imageView = vkengine::helper::resource::createImageView(
            ctx.getDevice()->logicaldevice,
            this->image,
            this->imageFormat,
            this->aspectFlags,
            1,
            1);

        this->descriptorCount = 1;
        this->update();
        this->barrierHelper.update(this->imageFormat, 1, 1);
    }

    void VKImage2D::updateBinding(VkDescriptorSetLayoutBinding &binding)
    {
        switch (this->descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
            binding.descriptorType = this->descriptorType;
            binding.descriptorCount = this->descriptorCount;
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = this->stageFlags;
            break;
        default:
            EXIT_TO_LOGGER("Descriptor Type Image");
            break;
        }
    }

    void VKImage2D::updateWrite(VkWriteDescriptorSet &write)
    {
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstSet = VK_NULL_HANDLE; // Will be set by DescriptorSet::create()
        write.dstBinding = 0;          // Will be set by DescriptorSet::create()
        write.dstArrayElement = 0;
        write.descriptorType = this->descriptorType;
        write.descriptorCount = this->descriptorCount;
        write.pImageInfo = &this->imageInfo;
        write.pBufferInfo = nullptr;
        write.pTexelBufferView = nullptr;
    }

    void VKImage2D::cleanup()
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


        this->barrierHelper.Currentlayout() = VK_IMAGE_LAYOUT_UNDEFINED;
        this->barrierHelper.Currentaccess() = VK_ACCESS_2_NONE;
        this->barrierHelper.Currentstage() = VK_PIPELINE_STAGE_2_NONE;
    }

}
