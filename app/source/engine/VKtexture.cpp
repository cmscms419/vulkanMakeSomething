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

// https://github.com/SaschaWillems/Vulkan 이 사이트에서 texturearray 참고
// 해당 예제는 리소스가 배열 형태로 되어 있어서, 배열 형태로 텍스처를 로드하는 예제이다.
// 이 예제는 단일 리소스 여러개를 가져오는 형태로 여러번 리소스를 가져온다.
void Vk2DTextureArray::createTextureArrayImages(int type, std::vector<std::string>& texPath)
{
    this->imageCount = static_cast<uint32_t>(texPath.size());
    
    std::vector<VkBuffer> stagingBuffers;
    std::vector<VkDeviceMemory> stagingBufferMemorys;
    this->VKmipLevels = 1;

    stagingBuffers.resize(this->imageCount);
    stagingBufferMemorys.resize(this->imageCount);

    VkDeviceSize offset = 0;

    // 1. 먼저 각 이미지의 크기를 계산하여 총 크기를 구합니다.
    VkDeviceSize totalImageSize = 0;
    std::vector<VkDeviceSize> imageSizes(this->imageCount);

    for (size_t i = 0; i < this->imageCount; i++) {
        // 임시로 이미지를 로드하여 크기를 얻습니다.
        GetTextureSize(texPath[i].c_str(), &texWidth, &texHeight);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        imageSizes[i] = imageSize;
        totalImageSize += imageSize;
    }

    // 2. 전체 크기를 기준으로 단일 스테이징 버퍼 생성
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

    // 3. 각 이미지 데이터를 단일 버퍼에 연속적으로 복사합니다.
    for (size_t i = 0; i < this->imageCount; i++)
    {
        unsigned char* pixels = load_png_rgba(texPath[i].c_str(), &texWidth, &texHeight, type);
        _CHECK_RESULT_((pixels != nullptr));

        // imageSizes[i]는 현재 이미지의 크기입니다.
        vkengine::helper::copyToDeviceMemory(
            this->device->logicaldevice,
            pixels,
            stagingBufferMemory,
            imageSizes[i],
            offset);

        free(pixels);
        offset += imageSizes[i];
    }

    // 4. 이미지 메모리 생성  
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
    // cube map texture = 6개의 이미지를 가지고 있어야 한다.
    uint32_t imageCount = 6;

    std::vector<VkBuffer> stagingBuffers;
    std::vector<VkDeviceMemory> stagingBufferMemorys;

    stagingBuffers.resize(imageCount);
    stagingBufferMemorys.resize(imageCount);

    VkDeviceSize offset = 0;

    // 1. 먼저 각 이미지의 크기를 계산하여 총 크기를 구합니다.
    VkDeviceSize totalImageSize = 0;
    std::vector<VkDeviceSize> imageSizes(imageCount);

    for (size_t i = 0; i < imageCount; i++) {
        // 임시로 이미지를 로드하여 크기를 얻습니다.
        GetTextureSize(texPath[i].c_str(), &texWidth, &texHeight);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        imageSizes[i] = imageSize;
        totalImageSize += imageSize;
    }
    this->VKmipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    // 2. 전체 크기를 기준으로 단일 스테이징 버퍼 생성
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

    // 3. 각 이미지 데이터를 단일 버퍼에 연속적으로 복사합니다.
    for (size_t i = 0; i < imageCount; i++)
    {
        unsigned char* pixels = load_png_rgba(texPath[i].c_str(), &texWidth, &texHeight, type);
        _CHECK_RESULT_((pixels != nullptr));

        // imageSizes[i]는 현재 이미지의 크기입니다.
        vkengine::helper::copyToDeviceMemory(
            this->device->logicaldevice,
            pixels,
            stagingBufferMemory,
            imageSizes[i],
            offset);

        free(pixels);
        offset += imageSizes[i];
    }

    // 4. 이미지 메모리 생성  
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

