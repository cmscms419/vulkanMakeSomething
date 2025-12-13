#include "VKSkyTexture.h"

using namespace vkengine::Log;

namespace vkengine {
    VKskyTexture::VKskyTexture(VKcontext& ctx) : ctx(ctx), prefiltered(ctx), irradiance(ctx), brdfLUT(ctx), samplerLinearRepeat(ctx), samplerLinearClamp(ctx)
    {
        samplerLinearRepeat.createLinearRepeat();
        samplerLinearClamp.createLinearClamp();
    }
    VKskyTexture::~VKskyTexture()
    {
        this->cleanup();
    }
    void VKskyTexture::LoadKTXMap(const cString& prefilteredFilename, const cString& irradianceFilename, const cString& brdfLutFileName)
    {
        PRINT_TO_LOGGER("Loading IBL textures...\n");
        PRINT_TO_LOGGER("  Prefiltered: %s\n", prefilteredFilename.c_str());
        PRINT_TO_LOGGER("  Irradiance: %s\n", irradianceFilename.c_str());
        PRINT_TO_LOGGER("  BRDF LUT: %s\n", brdfLutFileName.c_str());

        this->prefiltered.createTextureFromKtx2(prefilteredFilename, true);
        this->prefiltered.setSampler(this->samplerLinearRepeat.getSampler());

        this->irradiance.createTextureFromKtx2(irradianceFilename, true);
        this->irradiance.setSampler(this->samplerLinearRepeat.getSampler());

        this->brdfLUT.createTextureFromImage(brdfLutFileName, false, true);
        this->brdfLUT.setSampler(this->samplerLinearClamp.getSampler());
    }
    void VKskyTexture::cleanup()
    {
        this->prefiltered.cleanup();
        this->irradiance.cleanup();
        this->brdfLUT.cleanup();
    }
}