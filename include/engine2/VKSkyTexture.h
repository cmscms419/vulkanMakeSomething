#ifndef INCLUDE_VK_SKY_TEXTURE_H_
#define INCLUDE_VK_SKY_TEXTURE_H_

#include "VKContext.h"
#include "VKImage2D.h"
#include "VKsampler.h"
#include "VKResourceBindingData.h"

namespace vkengine {
    class VKskyTexture
    {
    public:
        VKskyTexture(VKcontext& ctx);
        ~VKskyTexture();

        void LoadKTXMap(
            const cString& prefilteredFilename, 
            const cString& irradianceFilename,
            const cString& brdfLutFileName
        );

        VKImage2D& Prefiltered() {
            return this->prefiltered;
        };

        VKImage2D& Irradiance() {
            return this->irradiance;
        };

        VKImage2D& BrdfLUT() {
            return this->brdfLUT;
        };


        void cleanup();
    private:

        VKcontext& ctx;

        VKImage2D prefiltered; // Prefiltered environment map for specular
        VKImage2D irradiance;  // Convolved irradiance cubemap for diffuse
        VKImage2D brdfLUT;     // BRDF integration lookup texture

        VKSampler samplerLinearRepeat;
        VKSampler samplerLinearClamp;

        VkDescriptorSetLayout descriptorSetLayout_{ VK_NULL_HANDLE };
        VkDescriptorSet descriptorSet_{ VK_NULL_HANDLE };

    };

}

#endif INCLUDE_VK_SKY_TEXTURE_H_