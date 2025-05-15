#include "VKtexture.h"
#include "helper.h"

#define _IMAGE_LOAD_TYPE_ 0

#include "../../common/resourseload.h"


void Vk2DTexture::createTextureImage(int type, const char* texPath)
{
    unsigned char* pixels = load_png_rgba(texPath, &texWidth, &texHeight, type);
    
    _CHECK_RESULT_((pixels != nullptr));
    

    VkDeviceSize imageSize = texWidth * texHeight * 4;
    
    this->VKmipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    //this->VKmipLevels = 1;

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

    vkDestroyBuffer(this->device->logicaldevice, stagingBuffer, nullptr);
    vkFreeMemory(this->device->logicaldevice, stagingBufferMemory, nullptr);

    vkengine::helper::generateMipmaps(
        this->device->physicalDevice,
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        this->image,
        VK_FORMAT_R8G8B8A8_SRGB,
        texWidth,
        texHeight,
        this->VKmipLevels);

}

void Vk2DTexture::createTextureImageView(VkFormat format)
{
    this->imageView = vkengine::helper::createImageView(
        this->device->logicaldevice,
        this->image,
        format,
        VK_IMAGE_ASPECT_COLOR_BIT,
        this->VKmipLevels);
}

void Vk2DTexture::createTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(this->device->physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
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

    _VK_CHECK_RESULT_(vkCreateSampler(this->device->logicaldevice, &samplerInfo, nullptr, &this->sampler));
}

// https://github.com/SaschaWillems/Vulkan �� ����Ʈ���� texturearray ����
// �ش� ������ ���ҽ��� �迭 ���·� �Ǿ� �־, �迭 ���·� �ؽ�ó�� �ε��ϴ� �����̴�.
// �� ������ ���� ���ҽ� �������� �������� ���·� ������ ���ҽ��� �����´�.
void Vk2DTextureArray::createTextureArrayImages(int type, std::vector<std::string>& texPath)
{
    this->imageCount = static_cast<uint32_t>(texPath.size());
    
    std::vector<VkBuffer> stagingBuffers;
    std::vector<VkDeviceMemory> stagingBufferMemorys;
    this->VKmipLevels = 1;

    stagingBuffers.resize(this->imageCount);
    stagingBufferMemorys.resize(this->imageCount);

    VkDeviceSize offset = 0;

    // 1. ���� �� �̹����� ũ�⸦ ����Ͽ� �� ũ�⸦ ���մϴ�.
    VkDeviceSize totalImageSize = 0;
    std::vector<VkDeviceSize> imageSizes(this->imageCount);

    for (size_t i = 0; i < this->imageCount; i++) {
        // �ӽ÷� �̹����� �ε��Ͽ� ũ�⸦ ����ϴ�.
        GetTextureSize(texPath[i].c_str(), &texWidth, &texHeight);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        imageSizes[i] = imageSize;
        totalImageSize += imageSize;
    }

    // 2. ��ü ũ�⸦ �������� ���� ������¡ ���� ����
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
     
    vkengine::helper::createBuffer(
        this->device->logicaldevice,
        this->device->physicalDevice,
        totalImageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // 3. �� �̹��� �����͸� ���� ���ۿ� ���������� �����մϴ�.
    for (size_t i = 0; i < this->imageCount; i++)
    {
        unsigned char* pixels = load_png_rgba(texPath[i].c_str(), &texWidth, &texHeight, type);
        _CHECK_RESULT_((pixels != nullptr));

        // imageSizes[i]�� ���� �̹����� ũ���Դϴ�.
        vkengine::helper::copyToDeviceMemory(
            this->device->logicaldevice,
            pixels,
            stagingBufferMemory,
            imageSizes[i],
            offset);

        free(pixels);
        offset += imageSizes[i];
    }

    // 4. �̹��� �޸� ����  
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
        this->imageMemory,
        this->imageCount);

    vkengine::helper::transitionImageLayout2(
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        this->image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        this->VKmipLevels,
        this->imageCount);

    vkengine::helper::copyBufferToImage2(
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        stagingBuffer,
        this->image,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight),
        imageSizes);

    vkengine::helper::transitionImageLayout2(
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        this->image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        this->VKmipLevels,
        this->imageCount);

    vkDestroyBuffer(this->device->logicaldevice, stagingBuffer, nullptr);
    vkFreeMemory(this->device->logicaldevice, stagingBufferMemory, nullptr);

}

void Vk2DTextureArray::createTextureImageView(VkFormat format)
{
    this->imageView = vkengine::helper::createArrayImageView(
        this->device->logicaldevice,
        this->image,
        format,
        VK_IMAGE_ASPECT_COLOR_BIT,
        this->VKmipLevels,
        this->imageCount);
}

void Vk2DTextureArray::createTextureSampler()
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

    _VK_CHECK_RESULT_(vkCreateSampler(this->device->logicaldevice, &samplerInfo, nullptr, &this->sampler));
}

void VKcubeMap::createTextureCubeImages(int type, std::vector<std::string>& texPath)
{
    // cube map texture = 6���� �̹����� ������ �־�� �Ѵ�.
    uint32_t imageCount = 6;

    std::vector<VkBuffer> stagingBuffers;
    std::vector<VkDeviceMemory> stagingBufferMemorys;

    stagingBuffers.resize(imageCount);
    stagingBufferMemorys.resize(imageCount);

    VkDeviceSize offset = 0;

    // 1. ���� �� �̹����� ũ�⸦ ����Ͽ� �� ũ�⸦ ���մϴ�.
    VkDeviceSize totalImageSize = 0;
    std::vector<VkDeviceSize> imageSizes(imageCount);

    for (size_t i = 0; i < imageCount; i++) {
        // �ӽ÷� �̹����� �ε��Ͽ� ũ�⸦ ����ϴ�.
        GetTextureSize(texPath[i].c_str(), &texWidth, &texHeight);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        imageSizes[i] = imageSize;
        totalImageSize += imageSize;
    }
    this->VKmipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    // 2. ��ü ũ�⸦ �������� ���� ������¡ ���� ����
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    vkengine::helper::createBuffer(
        this->device->logicaldevice,
        this->device->physicalDevice,
        totalImageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // 3. �� �̹��� �����͸� ���� ���ۿ� ���������� �����մϴ�.
    for (size_t i = 0; i < imageCount; i++)
    {
        unsigned char* pixels = load_png_rgba(texPath[i].c_str(), &texWidth, &texHeight, type);
        _CHECK_RESULT_((pixels != nullptr));

        // imageSizes[i]�� ���� �̹����� ũ���Դϴ�.
        vkengine::helper::copyToDeviceMemory(
            this->device->logicaldevice,
            pixels,
            stagingBufferMemory,
            imageSizes[i],
            offset);

        free(pixels);
        offset += imageSizes[i];
    }

    // 4. �̹��� �޸� ����  
    vkengine::helper::createImage2(
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
        this->imageMemory,
        imageCount,
        VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
        );

    vkengine::helper::transitionImageLayout2(
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        this->image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        this->VKmipLevels,
        imageCount);

    vkengine::helper::copyBufferToImage2(
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        stagingBuffer,
        this->image,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight),
        imageSizes);

    vkengine::helper::generateMipmapsCubeMap(
        this->device->physicalDevice,
        this->device->logicaldevice,
        this->device->commandPool,
        this->device->graphicsVKQueue,
        this->image,
        VK_FORMAT_R8G8B8A8_SRGB,
        texWidth,
        texHeight,
        this->VKmipLevels);

    vkDestroyBuffer(this->device->logicaldevice, stagingBuffer, nullptr);
    vkFreeMemory(this->device->logicaldevice, stagingBufferMemory, nullptr);
}

void VKcubeMap::createTextureImageView(VkFormat format)
{
    this->imageView = vkengine::helper::createCubeImageView(
        this->device->logicaldevice,
        this->image,
        format,
        VK_IMAGE_ASPECT_COLOR_BIT,
        this->VKmipLevels);
}

void VKcubeMap::createTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(this->device->physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = this->VKmipLevels;

    _VK_CHECK_RESULT_(vkCreateSampler(this->device->logicaldevice, &samplerInfo, nullptr, &this->sampler));
}

