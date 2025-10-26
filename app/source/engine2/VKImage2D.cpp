#include "VKImage2D.h"

using namespace vkengine::Log;

namespace vkengine
{
    VKImage2D::VKImage2D(VkContext& context) : ctx(context)
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
        this->getBarrierHelper().update(mipLevels, arrayLayers);
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
        VkDeviceSize imageSize = width * height * channels;

        StagingBuffer stagingBuffer(imageSize);

        // 스테이징 버퍼 생성
        stagingBuffer.device = ctx.getDevice()->logicaldevice;
        stagingBuffer.usageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        stagingBuffer.memoryPropertyFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBuffer.createBuffer(ctx.getDevice()->physicalDevice);

        // 스테이징 버퍼에 이미지 데이터 복사
        stagingBuffer.mapToMeBuffer(imageSize, 0);
        stagingBuffer.copyToMeBuffer(pixelData, imageSize);


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

        this->resourceBinding.image = this->image;
        this->resourceBinding.imageView = this->imageView;
        this->resourceBinding.descriptorCount = 1;
        this->resourceBinding.update();
        this->getBarrierHelper().update(1, 1);

        VkCommandBuffer cmb = this->getBarrierHelper().beginSingleTimeCommands2(
            ctx.getDevice()->logicaldevice,
            ctx.getDevice()->transferCommandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // 이미지 레이아웃 전환 (UNDEFINED -> TRANSFER_DST_OPTIMAL)
        this->getBarrierHelper().transitionImageLayout2(
            cmb,
            this->image,
            imageFormat,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT);

        vkengine::helper::copyBufferToImage3(
            cmb,
            stagingBuffer.buffer,
            this->image,
            width,
            height);

        this->getBarrierHelper().transitionImageLayout2(
            cmb,
            this->image,
            imageFormat,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_2_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

        this->getBarrierHelper().endSingleTimeCommands2(
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