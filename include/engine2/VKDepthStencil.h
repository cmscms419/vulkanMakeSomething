#ifndef VK_DEPTHSTENCIL_INCLUDE_H_
#define VK_DEPTHSTENCIL_INCLUDE_H_

#include "VKContext.h"
#include "VKbarrier2.h"

namespace vkengine {
    class VKDepthStencil
    {
    public:
        VKDepthStencil(VKcontext& ctx);
        ~VKDepthStencil();

        void create(cUint32_t width, cUint32_t height, VkSampleCountFlagBits msaaSamples);
        void cleanup();

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
