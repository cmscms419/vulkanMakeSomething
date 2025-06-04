#include "VKtexture.h"
#include "helper.h"

namespace vkengine {

    void Vk2DTexture::createTextureImage()
    {
        uint32_t texWidth = this->resource->texWidth;
        uint32_t texHeight = this->resource->texHeight;
        uint32_t texChannels = this->resource->texChannels;
        cUChar* pixels = this->resource->data;

        if (texWidth == 0 || texHeight == 0) {
            return;
        }

        VkDeviceSize imageSize = texWidth * texHeight * texChannels;

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
            texWidth,
            texHeight);

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
    void Vk2DTextureArray::createTextureImage()
    {
        std::vector<VkBuffer> stagingBuffers;
        std::vector<VkDeviceMemory> stagingBufferMemorys;

        stagingBuffers.resize(this->imageCount);
        stagingBufferMemorys.resize(this->imageCount);

        VkDeviceSize offset = 0;

        // 1. 먼저 각 이미지의 크기를 계산하여 총 크기를 구합니다.
        VkDeviceSize totalImageSize = 0;
        std::vector<VkDeviceSize> imageSizes(this->imageCount);

        for (size_t i = 0; i < this->imageCount; i++) {
            TextureResource& resource = this->resource[i];
            VkDeviceSize imageSize = resource.texWidth * resource.texHeight * resource.texChannels;
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
            TextureResource& resource = this->resource[i];

            _CHECK_RESULT_((resource.data != nullptr));

            // imageSizes[i]는 현재 이미지의 크기입니다.
            vkengine::helper::copyToDeviceMemory(
                this->device->logicaldevice,
                resource.data,
                stagingBufferMemory,
                imageSizes[i],
                offset);

            offset += imageSizes[i];
        }

        // 가정 : 모든 이미지가 동일한 포맷과 크기를 가지고 있다고 가정합니다.
        // 이 예제에서는 VK_FORMAT_R8G8B8A8_SRGB 포맷을 사용합니다.
        uint32_t width = this->resource[0].texWidth;
        uint32_t height = this->resource[0].texHeight;
        uint32_t channels = this->resource[0].texChannels;

        // 4. 이미지 메모리 생성  
        vkengine::helper::createImage(
            this->device->logicaldevice,
            this->device->physicalDevice,
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
            width,
            height,
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

    void VKcubeMap::createTextureImage()
    {
        // cube map texture = 6개의 이미지를 가지고 있어야 한다.
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

        // 1. 먼저 각 이미지의 크기를 계산하여 총 크기를 구합니다.
        VkDeviceSize totalImageSize = 0;
        std::vector<VkDeviceSize> imageSizes(imageCount);

        for (size_t i = 0; i < imageCount; i++) {
            // 임시로 이미지를 로드하여 크기를 얻습니다.
            TextureResource& resource = this->resource[i];
            VkDeviceSize imageSize = resource.texWidth * resource.texHeight * resource.texChannels;
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
        for (size_t i = 0; i < imageCount; i++)
        {
            TextureResource& resource = this->resource[i];

            _CHECK_RESULT_((resource.data != nullptr));

            // imageSizes[i]는 현재 이미지의 크기입니다.
            vkengine::helper::copyToDeviceMemory(
                this->device->logicaldevice,
                resource.data,
                stagingBufferMemory,
                imageSizes[i],
                offset);

            offset += imageSizes[i];
        }

        // 가정 : 모든 이미지가 동일한 포맷과 크기를 가지고 있다고 가정합니다.
        // 이 예제에서는 VK_FORMAT_R8G8B8A8_SRGB 포맷을 사용합니다.
        uint32_t width = this->resource[0].texWidth;
        uint32_t height = this->resource[0].texHeight;
        uint32_t channels = this->resource[0].texChannels;

        // 4. 이미지 메모리 생성  
        vkengine::helper::createImage2(
            this->device->logicaldevice,
            this->device->physicalDevice,
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
            width,
            height,
            imageSizes);

        vkengine::helper::generateMipmapsCubeMap(
            this->device->physicalDevice,
            this->device->logicaldevice,
            this->device->commandPool,
            this->device->graphicsVKQueue,
            this->image,
            VK_FORMAT_R8G8B8A8_SRGB,
            width,
            height,
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

    void VkTextureBase::setResource(TextureResource* resource)
    {
        if (resource == nullptr) {
            _PRINT_TO_CONSOLE_("Resource is null.");
            return;
        }

        if (this->imageCount >= 6) {
            _PRINT_TO_CONSOLE_("Cube map texture can only have 6 images.");
            return;
        }

        if (this->imageCount == 0) {
            this->imageCount = 1; // 첫 번째 리소스만 설정
            this->resource = (TextureResource*)calloc(this->imageCount, sizeof(TextureResource));

            if (this->resource == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to allocate memory for resource.");
                return;
            }

            this->resource[0] = *resource;

            //this->VKmipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        }
        else {

            TextureResource* newData = (TextureResource*)calloc(this->imageCount + 1, sizeof(TextureResource));

            if (newData == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to allocate memory for new resource.");
                return;
            }

            for (size_t i = 0; i < this->imageCount; i++) {
                newData[i] = this->resource[i];
            }

            newData[this->imageCount] = *resource; // 마지막에 새로운 리소스 추가
            free(this->resource); // Free the old resource array
            this->resource = newData; // 새로운 리소스로 교체
            this->imageCount++;
        }
    }
}