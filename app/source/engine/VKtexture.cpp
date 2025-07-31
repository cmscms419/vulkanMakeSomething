#include "VKtexture.h"

namespace vkengine {

    void Vk2DTexture::createTextureImage()
    {
        uint32_t texWidth = this->resourcePNG->texWidth;
        uint32_t texHeight = this->resourcePNG->texHeight;
        uint32_t texChannels = this->resourcePNG->texChannels;
        cUChar* pixels = this->resourcePNG->data;

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
            this->image,
            this->imageMemory);

        vkengine::helper::transitionImageLayout(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            this->VKmipLevels);

        vkengine::helper::copyBufferToImage(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            stagingBuffer,
            this->image,
            texWidth,
            texHeight);

        vkDestroyBuffer(this->logicaldevice, stagingBuffer, nullptr);
        vkFreeMemory(this->logicaldevice, stagingBufferMemory, nullptr);

        vkengine::helper::generateMipmaps(
            this->physicalDevice,
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->image,
            VK_FORMAT_R8G8B8A8_SRGB,
            texWidth,
            texHeight,
            this->VKmipLevels);
    }

    void Vk2DTexture::createTextureImageView(VkFormat format)
    {
        this->imageView = vkengine::helper::createImageView(
            this->logicaldevice,
            this->image,
            format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            this->VKmipLevels);
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

        _VK_CHECK_RESULT_(vkCreateSampler(this->logicaldevice, &samplerInfo, nullptr, &this->sampler));
    }

    // https://github.com/SaschaWillems/Vulkan �� ����Ʈ���� texturearray ����
    // �ش� ������ ���ҽ��� �迭 ���·� �Ǿ� �־, �迭 ���·� �ؽ�ó�� �ε��ϴ� �����̴�.
    // �� ������ ���� ���ҽ� �������� �������� ���·� ������ ���ҽ��� �����´�.
    void Vk2DTextureArray::createTextureImage()
    {
        std::vector<VkBuffer> stagingBuffers;
        std::vector<VkDeviceMemory> stagingBufferMemorys;

        stagingBuffers.resize(this->imageCount);
        stagingBufferMemorys.resize(this->imageCount);

        VkDeviceSize offset = 0;

        // 1. ���� �� �̹����� ũ�⸦ ����Ͽ� �� ũ�⸦ ���մϴ�.
        VkDeviceSize totalImageSize = 0;
        std::vector<VkDeviceSize> imageSizes(this->imageCount);

        for (size_t i = 0; i < this->imageCount; i++) {
            TextureResourcePNG& resourcePNG = this->resourcePNG[i];
            VkDeviceSize imageSize = resourcePNG.texWidth * resourcePNG.texHeight * resourcePNG.texChannels;
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
        for (size_t i = 0; i < this->imageCount; i++)
        {
            TextureResourcePNG& resourcePNG = this->resourcePNG[i];

            _CHECK_RESULT_((resourcePNG.data != nullptr));

            // imageSizes[i]�� ���� �̹����� ũ���Դϴ�.
            vkengine::helper::copyToDeviceMemory(
                this->logicaldevice,
                resourcePNG.data,
                stagingBufferMemory,
                imageSizes[i],
                offset);

            offset += imageSizes[i];
        }

        // ���� : ��� �̹����� ������ ���˰� ũ�⸦ ������ �ִٰ� �����մϴ�.
        // �� ���������� VK_FORMAT_R8G8B8A8_SRGB ������ ����մϴ�.
        uint32_t width = this->resourcePNG[0].texWidth;
        uint32_t height = this->resourcePNG[0].texHeight;
        uint32_t channels = this->resourcePNG[0].texChannels;

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
            this->image,
            this->imageMemory,
            this->imageCount);

        vkengine::helper::transitionImageLayout(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            this->VKmipLevels,
            this->imageCount);

        vkengine::helper::copyBufferToImage2(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            stagingBuffer,
            this->image,
            width,
            height,
            imageSizes);

        vkengine::helper::transitionImageLayout(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            this->VKmipLevels,
            this->imageCount);

        vkDestroyBuffer(this->logicaldevice, stagingBuffer, nullptr);
        vkFreeMemory(this->logicaldevice, stagingBufferMemory, nullptr);

    }

    void Vk2DTextureArray::createTextureImageView(VkFormat format)
    {
        this->imageView = vkengine::helper::createArrayImageView(
            this->logicaldevice,
            this->image,
            format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            this->VKmipLevels,
            this->imageCount);
    }

    void Vk2DTextureArray::createTextureSampler()
    {
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

        _VK_CHECK_RESULT_(vkCreateSampler(this->logicaldevice, &samplerInfo, nullptr, &this->sampler));
    }

    void VKcubeMap::createTextureImage()
    {
        // cube map texture = 6���� �̹����� ������ �־�� �Ѵ�.
        if (this->imageCount != 6)
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
            TextureResourcePNG& resourcePNG = this->resourcePNG[i];
            VkDeviceSize imageSize = resourcePNG.texWidth * resourcePNG.texHeight * resourcePNG.texChannels;
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
            TextureResourcePNG& resourcePNG = this->resourcePNG[i];

            _CHECK_RESULT_((resourcePNG.data != nullptr));

            // imageSizes[i]�� ���� �̹����� ũ���Դϴ�.
            vkengine::helper::copyToDeviceMemory(
                this->logicaldevice,
                resourcePNG.data,
                stagingBufferMemory,
                imageSizes[i],
                offset);

            offset += imageSizes[i];
        }

        // ���� : ��� �̹����� ������ ���˰� ũ�⸦ ������ �ִٰ� �����մϴ�.
        // �� ���������� VK_FORMAT_R8G8B8A8_SRGB ������ ����մϴ�.
        uint32_t width = this->resourcePNG[0].texWidth;
        uint32_t height = this->resourcePNG[0].texHeight;
        uint32_t channels = this->resourcePNG[0].texChannels;

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
            this->image,
            this->imageMemory,
            imageCount,
            VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
        );

        vkengine::helper::transitionImageLayout(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->image,
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
            this->image,
            width,
            height,
            imageSizes);

        vkengine::helper::generateMipmapsCubeMap(
            this->physicalDevice,
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->image,
            VK_FORMAT_R8G8B8A8_SRGB,
            width,
            height,
            this->VKmipLevels);

        vkDestroyBuffer(this->logicaldevice, stagingBuffer, nullptr);
        vkFreeMemory(this->logicaldevice, stagingBufferMemory, nullptr);

    }

    void VKcubeMap::createTextureImgae2(VkFormat format)
    {
        // 1. KTX �ؽ�ó�� �����Ϳ� ũ�⸦ ��������
        TextureResourceKTX* resource = this->resourceKTX;
        _CHECK_RESULT_((resource != nullptr));

        ktx_uint8_t* ktxTextureData{};
        ktxTextureData = ktxTexture_GetData(resource->texture); // KTX �ؽ�ó ������ ��������
        
        ktx_uint32_t ktxTextureSize{};
        ktxTextureSize = ktxTexture_GetDataSize(resource->texture); // KTX �ؽ�ó ������ ũ�� ��������

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
            this->image,
            this->imageMemory,
            6, // Cube map�� 6���� ���� �����ϴ�.
            VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
        );

        vkengine::helper::transitionImageLayout(
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->image,
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
            this->image,
            width,
            height,
            this->VKmipLevels,
            this->resourceKTX->texture
        );

        vkengine::helper::generateMipmapsCubeMap(
            this->physicalDevice,
            this->logicaldevice,
            this->commandPool,
            this->graphicsVKQueue,
            this->image,
            format,
            width,
            height,
            this->VKmipLevels);

        vkDestroyBuffer(this->logicaldevice, stagingBuffer, nullptr);
        vkFreeMemory(this->logicaldevice, stagingMemory, nullptr);
    }

    void VKcubeMap::createTextureImageView(VkFormat format)
    {
        this->imageView = vkengine::helper::createCubeImageView(
            this->logicaldevice,
            this->image,
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
        samplerInfo.maxLod = this->VKmipLevels;

        _VK_CHECK_RESULT_(vkCreateSampler(this->logicaldevice, &samplerInfo, nullptr, &this->sampler));
    }

    void VkTextureBase::setResourcePNG(TextureResourcePNG* resourcePNG)
    {
        if (resourcePNG == nullptr) {
            _PRINT_TO_CONSOLE_("Resource is null.");
            return;
        }

        if (this->imageCount == 0) {
            this->imageCount = 1; // ù ��° ���ҽ��� ����
            this->VKmipLevels = 1;
            //this->VKmipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

            this->resourcePNG = (TextureResourcePNG*)calloc(this->imageCount, sizeof(TextureResourcePNG));

            if (this->resourcePNG == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to allocate memory for resourcePNG.");
                return;
            }

            this->resourcePNG[0] = *resourcePNG;

        }
        else {
#if 0
            TextureResourcePNG* newData = (TextureResourcePNG*)calloc(this->imageCount + 1, sizeof(TextureResourcePNG));

            if (newData == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to allocate memory for new resourcePNG.");
                return;
            }

            for (size_t i = 0; i < this->imageCount; i++) {
                newData[i] = this->resourcePNG[i];
            }

            newData[this->imageCount] = *resourcePNG; // �������� ���ο� ���ҽ� �߰�
            free(this->resourcePNG); // Free the old resourcePNG array
            this->resourcePNG = newData; // ���ο� ���ҽ��� ��ü
            this->imageCount++;
#endif
            TextureResourcePNG* newData = this->resourcePNG;

            if ((this->resourcePNG = (TextureResourcePNG*)realloc(this->resourcePNG, sizeof(TextureResourcePNG) * (this->imageCount + 1))) == NULL)
            {
                _PRINT_TO_CONSOLE_("Failed to reallocate memory for resourcePNG.");
                free(newData);
                return;
            }
            this->resourcePNG[this->imageCount] = *resourcePNG; // �������� ���ο� ���ҽ� �߰�
            this->imageCount++;

        }
    }
}