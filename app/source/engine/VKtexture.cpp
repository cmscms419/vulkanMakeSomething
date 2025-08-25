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

        // 2D Array Texture�� ���� ���� �̹����� �ϳ��� �迭�� ó���մϴ�.
        this->imageData.reserve(1); // 2D Array Texture�� ���� �̹��� �����ͷ� ó���մϴ�.
        this->imageData.push_back(VKimageData{});

        // 1. ���� �� �̹����� ũ�⸦ ����Ͽ� �� ũ�⸦ ���մϴ�.
        VkDeviceSize totalImageSize = 0;
        std::vector<VkDeviceSize> imageSizes{};
        cUint32_t imageCount = static_cast<cUint32_t>(this->imageTextureDatas.size());

        for (auto& data : this->imageTextureDatas) {
            TextureResourcePNG* resource = data.resourcePNG;

            VkDeviceSize imageSize = resource->texWidth * resource->texHeight * resource->texChannels;

            imageSizes.push_back(imageSize);
            totalImageSize += imageSize;
        }

        // 2. ��ü ũ�⸦ �������� ���� ������¡ ���� ����
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

        // 3. �� �̹��� �����͸� ���� ���ۿ� ���������� �����մϴ�.
        for (size_t i = 0; i < imageCount; i++)
        {
            TextureResourcePNG* resourcePNG = this->imageTextureDatas[i].resourcePNG;

            _CHECK_RESULT_((resourcePNG->data != nullptr));

            // imageSizes[i]�� ���� �̹����� ũ���Դϴ�.
            vkengine::helper::copyToDeviceMemory(
                this->logicaldevice,
                resourcePNG->data,
                stagingBufferMemory,
                imageSizes[i],
                offset);

            offset += imageSizes[i];
        }

        // ���� : ��� �̹����� ������ ���˰� ũ�⸦ ������ �ִٰ� �����մϴ�.
        // �� ���������� VK_FORMAT_R8G8B8A8_SRGB ������ ����մϴ�.
        uint32_t width = this->imageTextureDatas[0].resourcePNG->texWidth;
        uint32_t height = this->imageTextureDatas[0].resourcePNG->texHeight;
        uint32_t channels = this->imageTextureDatas[0].resourcePNG->texChannels;

        // 4. �̹��� �޸� ����  
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
        VKimageData& data = this->imageData[0]; // 2D Array Texture�� ���� �̹��� �����ͷ� ó���մϴ�.

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
        VKimageData& data = this->imageData[0]; // 2D Array Texture�� ���� �̹��� �����ͷ� ó���մϴ�.

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
        // cube map texture = 6���� �̹����� ������ �־�� �Ѵ�.
        cUint32_t imageCount = static_cast<cUint32_t>(this->imageTextureDatas.size());
        this->imageData.reserve(1); // 2D Array Texture�� ���� �̹��� �����ͷ� ó���մϴ�.
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

        // 1. ���� �� �̹����� ũ�⸦ ����Ͽ� �� ũ�⸦ ���մϴ�.
        VkDeviceSize totalImageSize = 0;
        std::vector<VkDeviceSize> imageSizes(imageCount);

        for (size_t i = 0; i < imageCount; i++) {
            // �ӽ÷� �̹����� �ε��Ͽ� ũ�⸦ ����ϴ�.
            TextureResourcePNG* resourcePNG = this->imageTextureDatas[i].resourcePNG;
            VkDeviceSize imageSize = resourcePNG->texWidth * resourcePNG->texHeight * resourcePNG->texChannels;
            imageSizes[i] = imageSize;
            totalImageSize += imageSize;
        }

        // 2. ��ü ũ�⸦ �������� ���� ������¡ ���� ����
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

        // 3. �� �̹��� �����͸� ���� ���ۿ� ���������� �����մϴ�.
        for (size_t i = 0; i < imageCount; i++)
        {
            TextureResourcePNG* resourcePNG = this->imageTextureDatas[i].resourcePNG;

            _CHECK_RESULT_((resourcePNG->data != nullptr));

            // imageSizes[i]�� ���� �̹����� ũ���Դϴ�.
            vkengine::helper::copyToDeviceMemory(
                this->logicaldevice,
                resourcePNG->data,
                stagingBufferMemory,
                imageSizes[i],
                offset);

            offset += imageSizes[i];
        }

        // ���� : ��� �̹����� ������ ���˰� ũ�⸦ ������ �ִٰ� �����մϴ�.
        // �� ���������� VK_FORMAT_R8G8B8A8_SRGB ������ ����մϴ�.
        uint32_t width = this->imageTextureDatas[0].resourcePNG->texWidth;
        uint32_t height = this->imageTextureDatas[0].resourcePNG->texHeight;
        uint32_t channels = this->imageTextureDatas[0].resourcePNG->texChannels;

        // 4. �̹��� �޸� ����  
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
        // 1. KTX �ؽ�ó�� �����Ϳ� ũ�⸦ ��������
        TextureResourceKTX* resource = this->imageTextureDatas[0].resourceKTX;
        this->imageData.reserve(1); // Cube map�� ���� �̹��� �����ͷ� ó���մϴ�.
        this->imageData.push_back(VKimageData{});

        _CHECK_RESULT_((resource != nullptr));

        ktx_uint8_t* ktxTextureData{};
        ktxTextureData = ktxTexture_GetData(resource->texture); // KTX �ؽ�ó ������ ��������

        ktx_uint32_t ktxTextureSize{};
        ktxTextureSize = static_cast<ktx_uint32_t>(ktxTexture_GetDataSize(resource->texture)); // KTX �ؽ�ó ������ ũ�� ��������

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        // 2. KTX �ؽ�ó ũ�⸸ŭ ������¡ ���� ����
        vkengine::helper::createBuffer(
            this->logicaldevice,
            this->physicalDevice,
            ktxTextureSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingMemory);

        // 3. ������¡ ���ۿ� KTX �ؽ�ó ������ ����
        vkengine::helper::copyToDeviceMemory(
            this->logicaldevice,
            ktxTextureData,
            stagingMemory,
            ktxTextureSize);

        // 4. KTX �ؽ�ó�� �ʺ�, ����, ä�� �� ��������
        cUint32_t mipLevels = resource->texture->numLevels;
        cUint32_t width = resource->texWidth;
        cUint32_t height = resource->texHeight;
        cUint32_t channels = resource->texChannels;

        this->VKmipLevels = mipLevels;

        // 5. �̹��� �޸� ����
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
            6, // Cube map�� 6���� ���� �����ϴ�.
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
            6); // Cube map�� 6���� ���� �����ϴ�.

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