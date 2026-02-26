#ifndef VK_SHADOWMAP_INCLUDE_H_
#define VK_SHADOWMAP_INCLUDE_H_

#include "VKContext.h"
#include "VKResourceBindingData.h"

namespace vkengine
{
    class VKShadowMap
    {

    public:
        VKShadowMap(VKcontext &ctx);
        ~VKShadowMap();
        VkImage getImage();
        VkImageView getImageView();
        cUint32_t getWidth();
        cUint32_t getHeight();
        VKResourceBinding &getResouceBinding();

    private:
        VKcontext &ctx;
        VkImage image;
        VkDeviceMemory memory;
        VkImageView imageView;
        VkSampler sampler;
        cUint32_t width;
        cUint32_t height;
        VkFormat format;
        VKResourceBinding resourceBinding;
    };
}

#endif // !VK_SHADOWMAP_INCLUDE_H_
