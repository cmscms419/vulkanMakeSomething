#ifndef INCLUDE_VK_SAMPLER_H_
#define INCLUDE_VK_SAMPLER_H_

#include "VKContext.h"

namespace vkengine
{
    class VKSamplerHandler
    {
    public:
        VKSamplerHandler(VKcontext& context);
        ~VKSamplerHandler();
        VkSampler getSampler() const;
        
        void createAnisoRepeat();
        void createAnisoClamp();
        void createLinearRepeat();
        void createLinearClamp();

        void cleanup();

    private:
        VKcontext& ctx;
        VkSampler sampler;

    };

}


#endif // !INCLUDE_VK_SAMPLER_H_
