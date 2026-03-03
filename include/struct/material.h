#ifndef INCLUDE_MATERIAL_H_
#define INCLUDE_MATERIAL_H_

#include "common.h"

struct cMaterial
{
    alignas(16) cVec4 emissiveFactor = cVec4(0.0f);
    alignas(16) cVec4 baseColorFactor = cVec4(1.0f);
    alignas(4) cFloat roughness = 1.0f;
    alignas(4) cFloat transparencyFactor = 1.0f;
    alignas(4) cFloat discardAlpha = 0.0f;
    alignas(4) cFloat metallicFactor = 0.0f;
    alignas(4) cInt baseColorTextureIndex = -1;
    alignas(4) cInt emissiveTextureIndex = -1;
    alignas(4) cInt normalTextureIndex = -1;
    alignas(4) cInt opacityTextureIndex = -1;
    alignas(4) cInt metallicRoughnessTextureIndex = -1;
    alignas(4) cInt occlusionTextureIndex = -1;
};

#endif // !INCLUDE_MATERIAL_H_