#include "VKImage2D.h"

using namespace vkengine::Log;

namespace vkengine
{

    std::string fixPath(const cString& path) // for linux path
    {
        std::string fixed = path;
        std::replace(fixed.begin(), fixed.end(), '\\', '/');
        return fixed;
    }

    VKImage2D::VKImage2D(VKcontext& context) : ctx(context)
    {
    }

    VKImage2D::VKImage2D(VKImage2D&& other) noexcept : ctx(other.ctx),
        image(other.image),
        imageMemory(other.imageMemory),
        imageView(other.imageView),
        imageFormat(other.imageFormat),
        width(other.width),
        height(other.height),
        usageFlags(other.usageFlags),
        aspectFlags(other.aspectFlags)
    {
        other.image = VK_NULL_HANDLE;
        other.imageMemory = VK_NULL_HANDLE;
        other.imageView = VK_NULL_HANDLE;
        other.imageFormat = VK_FORMAT_UNDEFINED;
        other.width = 0;
        other.height = 0;
        other.usageFlags = 0;
        other.aspectFlags = 0;
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
        uint32_t mipLevels,
        uint32_t arrayLayers,
        VkImageCreateFlagBits flags)
    {
        this->cleanup();

        // VKimage2D 이미지 생성
        vkengine::helper::createImage2(
            ctx.getDevice()->logicaldevice,
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
            this->imageMemory,
            arrayLayers,
            flags
        );

        this->imageView = vkengine::helper::createImageView(
            ctx.getDevice()->logicaldevice,
            this->image,
            format,
            aspectMask,
            mipLevels,
            arrayLayers);

        this->resourceBinding.image = this->image;
        this->resourceBinding.imageView = this->imageView;
        this->resourceBinding.descriptorCount = 1;
        this->resourceBinding.update();
        this->getBarrierHelper().update(this->image, this->imageFormat, mipLevels, arrayLayers);
    }

    void VKImage2D::createCubeImage(cUint32_t width, cUint32_t height, VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage, VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t arrayLayers, VkImageCreateFlagBits flags)
    {
        this->cleanup();

        // VKimage2D 이미지 생성
        vkengine::helper::createImage2(
            ctx.getDevice()->logicaldevice,
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
            this->imageMemory,
            arrayLayers,
            flags
        );

        this->imageView = vkengine::helper::createCubeImageView(
            ctx.getDevice()->logicaldevice,
            this->image,
            format,
            aspectMask,
            mipLevels);

        this->resourceBinding.image = this->image;
        this->resourceBinding.imageView = this->imageView;
        this->resourceBinding.descriptorCount = 1;
        this->resourceBinding.update();
        this->getBarrierHelper().update(this->image, this->imageFormat, mipLevels, arrayLayers);
    }

    void VKImage2D::createTextureFromKtx2(cString filepath, cBool usCubemap)
    {
        filepath = fixPath(filepath);

        size_t extensionPos = filepath.find_last_of('.');
        cString extension = (extensionPos != cString::npos) ? filepath.substr(extensionPos) : "";
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extensionPos == cString::npos || (extension != ".ktx2")) {
            EXIT_TO_LOGGER("지원하지 않는 이미지 형식입니다: " + extension);
        }

        TextureResourceKTX* resource = nullptr;
        cUint32_t mipLevels = 1;
        VkFormat vkFormat = VK_FORMAT_UNDEFINED;
        ktxTexture* baseTexture = nullptr;
        ktx_uint8_t* ktxTextureData = nullptr;
        ktx_size_t ktxTextureSize = 0;
        cUint32_t layCounter = 0;

        if (extension == ".ktx2")
        {
            resource = new TextureResourceKTX();
            resource->createResource2(filepath);
        }
        else
        {
            EXIT_TO_LOGGER("지원하지 않는 이미지 형식입니다: \n" + extension);
        }

        if (!resource)
        {
            EXIT_TO_LOGGER("리소스가 생성되지 않았습니다 \n");
        }
        else
        {
            if (!resource->texture2)
            {
                delete resource;
                EXIT_TO_LOGGER("KTX2 텍스처 리소스가 유효하지 않습니다.\n");
            }

            mipLevels = resource->texture2 ? resource->texture2->numLevels : 1;
            vkFormat = ktxTexture2_GetVkFormat(resource->texture2);
            baseTexture = ktxTexture(resource->texture2);
            ktxTextureData = ktxTexture_GetData(baseTexture);
            ktxTextureSize = ktxTexture_GetDataSize(baseTexture);
            layCounter = usCubemap ? 6 : 1;

            if (mipLevels == 0) {
                delete resource;
                EXIT_TO_LOGGER("KTX2 텍스처의 mipLevels가 유효하지 않습니다.\n");
            }

            if (vkFormat == VK_FORMAT_UNDEFINED) {
                vkFormat = usCubemap ? VK_FORMAT_R16G16B16A16_SFLOAT : VK_FORMAT_R16G16_SFLOAT;
            }

            VkImageCreateFlagBits flags = static_cast<VkImageCreateFlagBits>(usCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0);
            VkImageViewType viewType = usCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;

            VKBaseBuffer2 stagingBuffer(this->ctx);
            stagingBuffer.createStagingBuffer(ktxTextureSize, ktxTextureData);

            if (usCubemap)
            {
                this->createCubeImage(
                    resource->texWidth,
                    resource->texHeight,
                    vkFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    mipLevels,
                    layCounter,
                    flags
                );
            }
            else
            {
                this->createImage(
                    resource->texWidth,
                    resource->texHeight,
                    vkFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    mipLevels,
                    layCounter,
                    flags
                );
            }

            VkCommandBuffer cmb = this->resourceBinding.getBarrierHelper().beginSingleTimeCommands2(
                ctx.getDevice()->logicaldevice,
                ctx.getDevice()->transferCommandPool,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY);

            resourceBinding.getBarrierHelper().transitionImageLayout2(
                cmb,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT);

            vkengine::helper::copyBufferToImageKTX2(
                cmb,
                stagingBuffer.Buffer(),
                this->image,
                resource->texWidth,
                resource->texHeight,
                mipLevels,
                baseTexture,
                usCubemap
            );

            // 이미지 레이아웃 전환 (TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL)
            resourceBinding.getBarrierHelper().transitionImageLayout2(
                cmb, 
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
                VK_ACCESS_2_SHADER_READ_BIT, 
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

            resourceBinding.getBarrierHelper().endSingleTimeCommands2(
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
            (extension != ".png" && extension != ".jpg" && extension != ".jpeg")) {
            EXIT_TO_LOGGER("지원하지 않는 이미지 형식입니다: " + extension);
        }

        if (usCubemap)
        {
            EXIT_TO_LOGGER("PNG, JPG는 큐브맵을 제공하지 않습니다.");
        }

        TextureResourceBase* resource = nullptr;

        if (extension == ".png") {
            resource = new TextureResourcePNG();
            resource->createResource(file);
        }
        else {
            EXIT_TO_LOGGER("지원하지 않는 이미지 형식입니다: " + extension);
        }

        if (resource != nullptr) {
            this->createTextureFromPixelData(resource->data, resource->texWidth, resource->texHeight, resource->texChannels, sRGB);
        }
        else {
            vkengine::Log::EXIT_TO_LOGGER("Failed to create texture: resource is null.");
        }

    }

    void VKImage2D::createTextureFromPixelData(cUChar* pixelData, cUint32_t width, cUint32_t height, cUint32_t channels, cBool sRGB)
    {
        if (!pixelData)
        {
            EXIT_TO_LOGGER("pixelData is nullptr\n");
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

        // 스테이징 버퍼에 이미지 데이터 복사
        VKBaseBuffer2 stagingBuffer(this->ctx);
        stagingBuffer.createStagingBuffer(imageSize, pixelData);

        this->createImage(
            width,
            height,
            imageFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            1,
            1,
            (VkImageCreateFlagBits)0
        );

        // 해당 내부에는 VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT -> 한번만 사용한다는 의미가 담긴 플로그가 있다.
        // 2번정도 사용하기 때문에 별도 생각해야함
        VkCommandBuffer cmb = this->resourceBinding.getBarrierHelper().beginSingleTimeCommands2(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->transferCommandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // 이미지 레이아웃 전환 (UNDEFINED -> TRANSFER_DST_OPTIMAL)
        resourceBinding.getBarrierHelper().transitionImageLayout2(
            cmb,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT);

        vkengine::helper::copyBufferToImage3(
            cmb,
            stagingBuffer.Buffer(),
            this->image,
            width,
            height);

        // 이미지 레이아웃 전환 (TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL)
        resourceBinding.getBarrierHelper().transitionImageLayout2(
            cmb,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_2_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

        resourceBinding.getBarrierHelper().endSingleTimeCommands2(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->transferCommandPool,
            ctx.getDevice()->transferVKQueue,
            cmb);
    }

    void VKImage2D::cleanup()
    {
        // Cleanup code for VKImage2D
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(ctx.getDevice()->logicaldevice, imageView, nullptr);
            imageView = VK_NULL_HANDLE;
        }

        if (image != VK_NULL_HANDLE) {
            vkDestroyImage(ctx.getDevice()->logicaldevice, image, nullptr);
            image = VK_NULL_HANDLE;
        }

        if (imageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(ctx.getDevice()->logicaldevice, imageMemory, nullptr);
            imageMemory = VK_NULL_HANDLE;
        }

    }

}