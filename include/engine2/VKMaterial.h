#ifndef VK_MATERIAL_INCLUDE_H_
#define VK_MATERIAL_INCLUDE_H_

#include "common.h"

#include "VKContext.h"
#include "VKSampler.h"
#include "VKImage2D.h"
#include "VKResourceBindingData.h"
#include "VKDescriptorManager2.h"
#include "VKUniformBuffer2.h"

namespace vkengine {

class VKMaterial
{
  public:
    VKMaterial()
    {
        ubo.emissiveFactor = cVec4(0.0f, 0.0f, 0.0f, 0.0f);
        ubo.baseColorFactor = cVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ubo.roughness = 1.0f;
        ubo.transparencyFactor = 1.0f;
        ubo.discardAlpha = 0.0f;
        ubo.metallicFactor = 0.0f;
        // TODO: maybe add occlusionStrength and normalScale

        // 텍스쳐 배열에 대한 인덱스
        ubo.baseColorTextureIndex = -1;
        ubo.emissiveTextureIndex = -1;
        ubo.normalTextureIndex = -1;
        ubo.opacityTextureIndex = -1;
        ubo.metallicRoughnessTextureIndex = -1;
        ubo.occlusionTextureIndex = -1;
        // ubo.flags = sCastShadow | sReceiveShadow;
    }

    enum Flags {
        sCastShadow = 0x1,
        sReceiveShadow = 0x2,
        sTransparent = 0x4,
    };

    cMaterial2 ubo;
    uint32_t flags = sCastShadow | sReceiveShadow;

    cString name;

    void loadFromCache(const cString& cachePath);
    void writeToCache(const cString& cachePath);

  private:
};

} // namespace vkengine

#endif // !VK_MATERIAL_INCLUDE_H_
