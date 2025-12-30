#ifndef VK_DEPTHSTENCIL_INCLUDE_H_
#define VK_DEPTHSTENCIL_INCLUDE_H_

#include "VKContext.h"
#include "VKbarrier2.h"

namespace vkengine {
    class VKDepthStencil
    {
    public:
        VKDepthStencil(VKcontext& ctx) : ctx(ctx)
        {
        }

        void create(cUint32_t width, cUint32_t height, VkSampleCountFlagBits msaaSamples)
        {
            VkDevice device = ctx.getDevice()->logicaldevice;

            VkImageCreateInfo imageCI{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
            imageCI.imageType = VK_IMAGE_TYPE_2D;
            imageCI.format = ctx.getDepthStencil()->depthFormat;
            imageCI.extent = { width, height, 1 };
            imageCI.mipLevels = 1;
            imageCI.arrayLayers = 1;
            imageCI.samples = msaaSamples;
            imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            _VK_CHECK_RESULT_(vkCreateImage(device, &imageCI, nullptr, &image));

            VkMemoryAllocateInfo memAlloc{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
            VkMemoryRequirements memReqs;
            vkGetImageMemoryRequirements(device, image, &memReqs);
            memAlloc.allocationSize = memReqs.size;
            memAlloc.memoryTypeIndex = ctx.getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            _VK_CHECK_RESULT_(vkAllocateMemory(device, &memAlloc, nullptr, &memory));
            _VK_CHECK_RESULT_(vkBindImageMemory(device, image, memory, 0));

            // Create depth-stencil view for rendering (includes both depth and stencil aspects)
            VkImageViewCreateInfo depthStencilViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            depthStencilViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depthStencilViewCI.format = ctx.getDepthStencil()->depthFormat;
            depthStencilViewCI.subresourceRange = {};
            depthStencilViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (ctx.getDepthStencil()->depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
                depthStencilViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            depthStencilViewCI.subresourceRange.baseMipLevel = 0;
            depthStencilViewCI.subresourceRange.levelCount = 1;
            depthStencilViewCI.subresourceRange.baseArrayLayer = 0;
            depthStencilViewCI.subresourceRange.layerCount = 1;
            depthStencilViewCI.image = image;
            _VK_CHECK_RESULT_(vkCreateImageView(device, &depthStencilViewCI, nullptr, &view));

            // Create depth-only view for shader sampling (only depth aspect)
            VkImageViewCreateInfo depthOnlyViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            depthOnlyViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depthOnlyViewCI.format = ctx.getDepthStencil()->depthFormat;
            depthOnlyViewCI.subresourceRange = {};
            depthOnlyViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Only depth
            depthOnlyViewCI.subresourceRange.baseMipLevel = 0;
            depthOnlyViewCI.subresourceRange.levelCount = 1;
            depthOnlyViewCI.subresourceRange.baseArrayLayer = 0;
            depthOnlyViewCI.subresourceRange.layerCount = 1;
            depthOnlyViewCI.image = image;
            _VK_CHECK_RESULT_(vkCreateImageView(device, &depthOnlyViewCI, nullptr, &samplerView));
        }

        ~VKDepthStencil() {
            this->cleanup();
        }

        void cleanup() {
            VkDevice device = this->ctx.getDevice()->logicaldevice;

            if (samplerView != VK_NULL_HANDLE) {
                vkDestroyImageView(device, samplerView, nullptr);
                samplerView = VK_NULL_HANDLE;
            }
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(device, view, nullptr);
                view = VK_NULL_HANDLE;
            }
            if (memory != VK_NULL_HANDLE) {
                vkFreeMemory(device, memory, nullptr);
                memory = VK_NULL_HANDLE;
            }
            if (image != VK_NULL_HANDLE) {
                vkDestroyImage(device, image, nullptr);
                image = VK_NULL_HANDLE;
            }
        }

        VkImage image{ VK_NULL_HANDLE };
        VkDeviceMemory memory{ VK_NULL_HANDLE };
        VkImageView view{ VK_NULL_HANDLE };
        VkImageView samplerView{ VK_NULL_HANDLE };
        VKBarrierHelper barrier;

    private:
        VKcontext& ctx;
        
    };
}

#endif // !VK_DEPTHSTENCIL_INCLUDE_H_
