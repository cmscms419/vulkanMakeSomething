#include "VKtexture.h"

namespace vkengine {

    void Vk2DTexture::createTextureImagePNG()
    {
        size_t textureCount = this->imageTextureDatas.size();
        this->imageData.reserve(textureCount);

        for (size_t i = 0; i < textureCount; i++)
        {
            this->imageData.push_back(VKimageData{});
            
            imageResource& data = this->imageTextureDatas[i];
            VKimageData& imageData = this->imageData[i];

            uint32_t texWidth = data.resourcePNG->texWidth;
            uint32_t texHeight = data.resourcePNG->texHeight;
            uint32_t texChannels = data.resourcePNG->texChannels;
            cUChar* pixels = data.resourcePNG->data;

            if (texWidth == 0 || texHeight == 0) {
                return;
            }

            VkDeviceSize imageSize = texWidth * texHeight * texChannels;

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            vkengine::helper::createBuffer(
                this->logicaldevice,
                this->physicalDevice,
                imageSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingBufferMemory);

            vkengine::helper::copyToDeviceMemory(
                this->logicaldevice,
                pixels,
                stagingBufferMemory,
                imageSize);

            vkengine::helper::createImage(
                this->logicaldevice,
                this->physicalDevice,
                texWidth,
                texHeight,
                this->VKmipLevels,
                VK_SAMPLE_COUNT_1_BIT,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                imageData.image,
                imageData.imageMemory);

            vkengine::helper::transitionImageLayout(
                this->logicaldevice,
                this->commandPool,
                this->graphicsVKQueue,
                imageData.image,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                this->VKmipLevels);

            vkengine::helper::copyBufferToImage(
                this->logicaldevice,
                this->commandPool,
                this->graphicsVKQueue,
                stagingBuffer,
                imageData.image,
                texWidth,
                texHeight);

            vkDestroyBuffer(this->logicaldevice, stagingBuffer, nullptr);
            vkFreeMemory(this->logicaldevice, stagingBufferMemory, nullptr);

            vkengine::helper::generateMipmaps(
                this->physicalDevice,
                this->logicaldevice,
                this->commandPool,
                this->graphicsVKQueue,
                imageData.image,
                VK_FORMAT_R8G8B8A8_SRGB,
                texWidth,
                texHeight,
                this->VKmipLevels);
        }
    }

    void Vk2DTexture::createTextureImageView(VkFormat format)
    {
        for (VKimageData& image : this->imageData)
        {
            image.imageView = vkengine::helper::createImageView(
                this->logicaldevice,
                image.image,
                format,
                VK_IMAGE_ASPECT_COLOR_BIT,
                this->VKmipLevels);
        }
    }

    void Vk2DTexture::createTextureSampler()
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);

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

        for (VKimageData& image : this->imageData)
        {
            _VK_CHECK_RESULT_(vkCreateSampler(this->logicaldevice, &samplerInfo, nullptr, &image.sampler));
        }
    }

    void Vk2DArrayTexture::createTextureImagePNG()
    {
        VkDeviceSize offset = 0;

        // 2D Array Texture는 여러 개의 이미지를 하나의 배열로 처리합니다.
        this->imageData.reserve(1); // 2D Array Texture는 단일 이미지 데이터로 처리합니다.
        this->imageData.push_back(VKimageData{});

        // 1. 먼저 각 이미지의 크기를 계산하여 총 크기를 구합니다.
        VkDeviceSize totalImageSize = 0;
        std::vector<VkDeviceSize> imageSizes{};
        cUint32_t imageCount = static_cast<cUint32_t>(this->imageTextureDatas.size());

        for (auto& data : this->imageTextureDatas) {
            TextureResourcePNG* resource = data.resourcePNG;

            VkDeviceSize imageSize = resource->texWidth * resource->texHeight * resource->texChannels;

            imageSizes.push_back(imageSize);
            totalImageSize += imageSize;
        }

        // 2. 전체 크기를 기준으로 단일 스테이징 버퍼 생성
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        vkengine::helper::createBuffer(
            this->logicaldevice,
            this->physicalDevice,
            totalImageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        // 3. 각 이미지 데이터를 단일 버퍼에 연속적으로 복사합니다.
        for (size_t i = 0; i < imageCount; i++)
        {
            TextureResourcePNG* resourcePNG = this->imageTextureDatas[i].resourcePNG;

            _CHECK_RESULT_((resourcePNG->data != nullptr));

            // imageSizes[i]는 현재 이미지의 크기입니다.
            vkengine::helper::copyToDeviceMemory(
                this->logicaldevice,
                resourcePNG->data,
                stagingBufferMemory,
                imageSizes[i],
                offset);

            offset += imageSizes[i];
        }

        // 가정 : 모든 이미지가 동일한 포맷과 크기를 가지고 있다고 가정합니다.
        // 이 예제에서는 VK_FORMAT_R8G8B8A8_SRGB 포맷을 사용합니다.
        uint32_t width = this->imageTextureDatas[0].resourcePNG->texWidth;
        uint32_t height = this->imageTextureDatas[0].resourcePNG->texHeight;
        uint32_t channels = this->imageTextureDatas[0].resourcePNG->texChannels;

        // 4. 이미지 메모리 생성  
        vkengine::helper::createImage(
            this->logicaldevice,
            this->physicalDevice,
            width,
            height,
            channels,
            VK_SAMPLE_COUNT_1_BIT,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            imageData[0].image,
            imageData[0].imageMemory,
            imageCount
        );

        vkengine::helper::transitionImageLayout(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            imageData[0].image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            this->VKmipLevels,
            imageCount);

        vkengine::helper::copyBufferToImage2(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            stagingBuffer,
            imageData[0].image,
            width,
            height,
            imageSizes);

        vkengine::helper::transitionImageLayout(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            imageData[0].image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            this->VKmipLevels,
            imageCount);

        vkDestroyBuffer(this->logicaldevice, stagingBuffer, nullptr);
        vkFreeMemory(this->logicaldevice, stagingBufferMemory, nullptr);
    }

    void Vk2DArrayTexture::createTextureImageView(VkFormat format)
    {
        cUint32_t imageCount = static_cast<cUint32_t>(this->imageTextureDatas.size());
        VKimageData& data = this->imageData[0]; // 2D Array Texture는 단일 이미지 데이터로 처리합니다.

        data.imageView = vkengine::helper::createArrayImageView(
            this->logicaldevice,
            data.image,
            format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            this->VKmipLevels,
            imageCount);
    }

    void Vk2DArrayTexture::createTextureSampler()
    {
        VKimageData& data = this->imageData[0]; // 2D Array Texture는 단일 이미지 데이터로 처리합니다.

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);

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

        _VK_CHECK_RESULT_(vkCreateSampler(this->logicaldevice, &samplerInfo, nullptr, &data.sampler));
    }

    void VKcubeMap::createTextureImagePNG()
    {
        // cube map texture = 6개의 이미지를 가지고 있어야 한다.
        cUint32_t imageCount = static_cast<cUint32_t>(this->imageTextureDatas.size());
        this->imageData.reserve(1); // 2D Array Texture는 단일 이미지 데이터로 처리합니다.
        this->imageData.push_back(VKimageData{});

        if (imageCount != 6)
        {
            _PRINT_TO_CONSOLE_("Cube map texture must have 6 images.");
            return;
        }

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
            TextureResourcePNG* resourcePNG = this->imageTextureDatas[i].resourcePNG;
            VkDeviceSize imageSize = resourcePNG->texWidth * resourcePNG->texHeight * resourcePNG->texChannels;
            imageSizes[i] = imageSize;
            totalImageSize += imageSize;
        }

        // 2. 전체 크기를 기준으로 단일 스테이징 버퍼 생성
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        vkengine::helper::createBuffer(
            this->logicaldevice,
            this->physicalDevice,
            totalImageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        // 3. 각 이미지 데이터를 단일 버퍼에 연속적으로 복사합니다.
        for (size_t i = 0; i < imageCount; i++)
        {
            TextureResourcePNG* resourcePNG = this->imageTextureDatas[i].resourcePNG;

            _CHECK_RESULT_((resourcePNG->data != nullptr));

            // imageSizes[i]는 현재 이미지의 크기입니다.
            vkengine::helper::copyToDeviceMemory(
                this->logicaldevice,
                resourcePNG->data,
                stagingBufferMemory,
                imageSizes[i],
                offset);

            offset += imageSizes[i];
        }

        // 가정 : 모든 이미지가 동일한 포맷과 크기를 가지고 있다고 가정합니다.
        // 이 예제에서는 VK_FORMAT_R8G8B8A8_SRGB 포맷을 사용합니다.
        uint32_t width = this->imageTextureDatas[0].resourcePNG->texWidth;
        uint32_t height = this->imageTextureDatas[0].resourcePNG->texHeight;
        uint32_t channels = this->imageTextureDatas[0].resourcePNG->texChannels;

        // 4. 이미지 메모리 생성  
        vkengine::helper::createImage2(
            this->logicaldevice,
            this->physicalDevice,
            width,
            height,
            this->VKmipLevels,
            VK_SAMPLE_COUNT_1_BIT,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->imageData[0].image,
            this->imageData[0].imageMemory,
            imageCount,
            VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
        );

        vkengine::helper::transitionImageLayout(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->imageData[0].image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            this->VKmipLevels,
            imageCount);

        vkengine::helper::copyBufferToImage2(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            stagingBuffer,
            this->imageData[0].image,
            width,
            height,
            imageSizes);

        vkengine::helper::generateMipmapsCubeMap(
            this->physicalDevice,
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->imageData[0].image,
            VK_FORMAT_R8G8B8A8_SRGB,
            width,
            height,
            this->VKmipLevels);

        vkDestroyBuffer(this->logicaldevice, stagingBuffer, nullptr);
        vkFreeMemory(this->logicaldevice, stagingBufferMemory, nullptr);

    }

    void VKcubeMap::createTextureImgaeKTX(VkFormat format)
    {
        // 1. KTX 텍스처의 데이터와 크기를 가져오기
        TextureResourceKTX* resource = this->imageTextureDatas[0].resourceKTX;
        this->imageData.reserve(1); // Cube map은 단일 이미지 데이터로 처리합니다.
        this->imageData.push_back(VKimageData{});

        _CHECK_RESULT_((resource != nullptr));

        ktx_uint8_t* ktxTextureData{};
        ktxTextureData = ktxTexture_GetData(resource->texture); // KTX 텍스처 데이터 가져오기

        ktx_uint32_t ktxTextureSize{};
        ktxTextureSize = static_cast<ktx_uint32_t>(ktxTexture_GetDataSize(resource->texture)); // KTX 텍스처 데이터 크기 가져오기

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        // 2. KTX 텍스처 크기만큼 스테이징 버퍼 생성
        vkengine::helper::createBuffer(
            this->logicaldevice,
            this->physicalDevice,
            ktxTextureSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingMemory);

        // 3. 스테이징 버퍼에 KTX 텍스처 데이터 복사
        vkengine::helper::copyToDeviceMemory(
            this->logicaldevice,
            ktxTextureData,
            stagingMemory,
            ktxTextureSize);

        // 4. KTX 텍스처의 너비, 높이, 채널 수 가져오기
        cUint32_t mipLevels = resource->texture->numLevels;
        cUint32_t width = resource->texWidth;
        cUint32_t height = resource->texHeight;
        cUint32_t channels = resource->texChannels;

        this->VKmipLevels = mipLevels;

        // 5. 이미지 메모리 생성
        vkengine::helper::createImage2(
            this->logicaldevice,
            this->physicalDevice,
            width,
            height,
            this->VKmipLevels,
            VK_SAMPLE_COUNT_1_BIT,
            format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->imageData[0].image,
            this->imageData[0].imageMemory,
            6, // Cube map은 6개의 면을 가집니다.
            VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
        );

        vkengine::helper::transitionImageLayout(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->imageData[0].image,
            format,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            this->VKmipLevels,
            6); // Cube map은 6개의 면을 가집니다.

        vkengine::helper::copyBufferToImageKTX(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            stagingBuffer,
            this->imageData[0].image,
            width,
            height,
            this->VKmipLevels,
            resource->texture
        );

        vkengine::helper::generateMipmapsCubeMap(
            this->physicalDevice,
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->imageData[0].image,
            format,
            width,
            height,
            this->VKmipLevels);

        vkDestroyBuffer(this->logicaldevice, stagingBuffer, nullptr);
        vkFreeMemory(this->logicaldevice, stagingMemory, nullptr);
    }

    void VKcubeMap::createTextureImageView(VkFormat format)
    {
        this->imageData[0].imageView = vkengine::helper::createCubeImageView(
            this->logicaldevice,
            this->imageData[0].image,
            format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            this->VKmipLevels);
    }

    void VKcubeMap::createTextureSampler()
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);

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
        samplerInfo.maxLod = static_cast<float>(this->VKmipLevels);

        _VK_CHECK_RESULT_(vkCreateSampler(this->logicaldevice, &samplerInfo, nullptr, &this->imageData[0].sampler));
    }
}