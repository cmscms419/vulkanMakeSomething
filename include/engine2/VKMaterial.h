#ifndef VK_MATERIAL_INCLUDE_H_
#define VK_MATERIAL_INCLUDE_H_

#include "common.h"
#include "material.h"

#include "VKContext.h"
#include "VKSamplerHandler.h"
#include "VKImage2D.h"
#include "VKResourceBindingData.h"
#include "VKDescriptorManager2.h"
#include "VKUniformBuffer2.h"

namespace vkengine {

class VKMaterial
{
  public:
  VKMaterial();
    enum Flags {
        sCastShadow = 0x1,
        sReceiveShadow = 0x2,
        sTransparent = 0x4,
    };

    cMaterial ubo;
    uint32_t flags = sCastShadow | sReceiveShadow;

    cString name;

    void loadFromCache(const cString& cachePath);
    void writeToCache(const cString& cachePath);

  private:
};

} // namespace vkengine

#endif // !VK_MATERIAL_INCLUDE_H_
