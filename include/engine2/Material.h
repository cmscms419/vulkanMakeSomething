#ifndef VK_MATERIAL_INCLUDE_H_
#define VK_MATERIAL_INCLUDE_H_

#include "common.h"
#include "materialData.h"

#include "Context.h"
#include "SamplerHandle.h"
#include "Image2D.h"
#include "DescriptorManager2.h"
#include "UniformBuffer2.h"

namespace vkengine {

class Material
{
  public:
  Material();
    enum Flags {
        CastShadow = 0x1,
        ReceiveShadow = 0x2,
        Transparent = 0x4,
    };

    MaterialData ubo;
    uint32_t flags = CastShadow | ReceiveShadow;

    cString name;

    void loadFromCache(const cString& cachePath);
    void writeToCache(const cString& cachePath);

  private:
};

} // namespace vkengine

#endif // !VK_MATERIAL_INCLUDE_H_
