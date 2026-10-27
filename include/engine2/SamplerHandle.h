#ifndef INCLUDE_VK_SAMPLER_H_
#define INCLUDE_VK_SAMPLER_H_

#include "Context.h"

namespace vkengine
{
    class SamplerHandle
    {
    public:
        SamplerHandle(Context& context);
        ~SamplerHandle();
        VkSampler getSampler() const;
        
        void createAnisoRepeat();
        void createAnisoClamp();
        void createLinearRepeat();
        void createLinearClamp();
        void createShadowMapSampler();

        void cleanup();

    private:
        Context& ctx;
        VkSampler sampler;

    };

}

#endif // !INCLUDE_VK_SAMPLER_H_
