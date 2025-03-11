#include "VKtexture.h"
#include "../../common/resourseload.h"

#include "helper.h"

void VkTexture_::cleanup()
{
    vkDestroySampler(device->logicaldevice, sampler, nullptr);
    vkDestroyImageView(device->logicaldevice, imageView, nullptr);
    vkDestroyImage(device->logicaldevice, image, nullptr);
    vkFreeMemory(device->logicaldevice, imageMemory, nullptr);
}

void Vk2DTexture_::createTextureImage(int type)
{
    unsigned char* pixels = load_png_rgba(texPath, &texWidth, &texHeight, type);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) { return; }

    //this->VKmipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    this->VKmipLevels = 1;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    vkengine::helper::createBuffer(
        this->device->logicaldevice,
        this->device->physicalDevice,
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    vkengine::helper::copyToDeviceMemory(
        this->device->logicaldevice,
        pixels,
        stagingBufferMemory,
        imageSize);

    free(pixels);

    vkengine::helper::createImage(
        this->device->logicaldevice,
        this->device->physicalDevice,
        texWidth,
        texHeight,
        this->VKmipLevels,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        this->image,
        this->imageMemory);

    vkengine::helper::transitionImageLayout(
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        this->image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        this->VKmipLevels);

    vkengine::helper::copyBufferToImage(
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        stagingBuffer,
        this->image,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight));
    vkengine::helper::transitionImageLayout(
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        this->image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        this->VKmipLevels);

    vkDestroyBuffer(this->device->logicaldevice, stagingBuffer, nullptr);
    vkFreeMemory(this->device->logicaldevice, stagingBufferMemory, nullptr);
}

void Vk2DTexture_::createTextureImageView(VkFormat format)
{
    this->imageView = vkengine::helper::createImageView(
        this->device->logicaldevice,
        this->image,
        format,
        VK_IMAGE_ASPECT_COLOR_BIT,
        this->VKmipLevels);
}

void Vk2DTexture_::createTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(this->device->physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VK_CHECK_RESULT(vkCreateSampler(this->device->logicaldevice, &samplerInfo, nullptr, &this->sampler));
}

