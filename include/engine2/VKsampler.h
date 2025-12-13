#ifndef INCLUDE_VK_SAMPLER_H_
#define INCLUDE_VK_SAMPLER_H_

#include "VKContext.h"

#include "log.h"

namespace vkengine
{
    class VKSampler
    {
    public:
        VKSampler(VKcontext& context) : ctx(context)
        {
            Log::PRINT_TO_LOGGER("VK Sampler created.\n");
        }
        ~VKSampler()
        {
            this->cleanup();
            Log::PRINT_TO_LOGGER("VK Sampler destroyed.\n");
        }
        VkSampler getSampler() const { return sampler; }
        
        void createAnisoRepeat();
        void createAnisoClamp();
        void createLinearRepeat();
        void createLinearClamp();

        void cleanup();

    private:
        VKcontext& ctx;
        VkSampler sampler = VK_NULL_HANDLE;

    };

}


#endif // !INCLUDE_VK_SAMPLER_H_
