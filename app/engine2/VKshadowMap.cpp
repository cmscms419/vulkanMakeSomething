#include "VKshadowMap.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{

    VKShadowMap::VKShadowMap(VKcontext &ctx) : ctx(ctx)
    {
        const VkDevice device = ctx.getDevice()->logicaldevice;
        image = VK_NULL_HANDLE;
        memory = VK_NULL_HANDLE;
        imageView = VK_NULL_HANDLE;
        sampler = VK_NULL_HANDLE;
        width = 2048 * 2; // Shadow map resolution
        height = 2048 * 2;
        format = VK_FORMAT_D16_UNORM;
        resourceBinding;

        // Create shadow map image
        VkImageCreateInfo imageCI{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = format;
        imageCI.extent = {width, height, 1};
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        _VK_CHECK_RESULT_(vkCreateImage(device, &imageCI, nullptr, &image));

        // Allocate memory
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device, image, &memReqs);

        VkMemoryAllocateInfo memAlloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex =
            ctx.getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        _VK_CHECK_RESULT_(vkAllocateMemory(device, &memAlloc, nullptr, &memory));
        _VK_CHECK_RESULT_(vkBindImageMemory(device, image, memory, 0));

        // Create image view
        VkImageViewCreateInfo viewCI{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewCI.image = image;
        viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCI.format = format;
        viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewCI.subresourceRange.baseMipLevel = 0;
        viewCI.subresourceRange.levelCount = 1;
        viewCI.subresourceRange.baseArrayLayer = 0;
        viewCI.subresourceRange.layerCount = 1;
        _VK_CHECK_RESULT_(vkCreateImageView(device, &viewCI, nullptr, &imageView));

        // Create sampler for shadow map sampling
        VkSamplerCreateInfo samplerCI{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.minFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.borderColor =
            VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; // Outside shadow map = fully lit
        samplerCI.compareEnable = VK_TRUE;
        samplerCI.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL; // For shadow comparison
        _VK_CHECK_RESULT_(vkCreateSampler(device, &samplerCI, nullptr, &sampler));

        resourceBinding.image = image;
        resourceBinding.imageView = imageView;
        resourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        resourceBinding.descriptorCount = 1;
        resourceBinding.setSampler(sampler);
        resourceBinding.getBarrierHelper().update(format, 1, 1);
    }

    VKShadowMap::~VKShadowMap()
    {
        if (this->image != VK_NULL_HANDLE)
        {
            vkDestroyImage(ctx.getDevice()->logicaldevice, image, nullptr);
            image = VK_NULL_HANDLE;
        }

        if (this->memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(ctx.getDevice()->logicaldevice, memory, nullptr);
            memory = VK_NULL_HANDLE;
        }

        if (this->imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(ctx.getDevice()->logicaldevice, imageView, nullptr);
            imageView = VK_NULL_HANDLE;
        }

        if (sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(ctx.getDevice()->logicaldevice, sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
    }

    VkImage VKShadowMap::getImage()
    {
        return this->image;
    }

    VkImageView VKShadowMap::getImageView()
    {
        return this->imageView;
    }

    cUint32_t VKShadowMap::getWidth()
    {
        return this->width;
    }
    
    cUint32_t VKShadowMap::getHeight()
    {
        return this->height;
    }
    
    VKResourceBinding &VKShadowMap::getResouceBinding()
    {
        return this->resourceBinding;
    }

}
