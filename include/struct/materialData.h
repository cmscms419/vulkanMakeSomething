#ifndef INCLUDE_MATERIAL_H_
#define INCLUDE_MATERIAL_H_

#include "common.h"

// 머티리얼이 어떤 PBR 워크플로로 저작되었는지.
// shader/materialCommon.glsl 의 MATERIAL_WORKFLOW_* define 과 값이 반드시 일치해야 한다.
enum class MaterialWorkflow : cInt
{
    MetallicRoughness = 0,  // glTF: metallicRoughnessTextureIndex = (G: roughness, B: metallic)
    SpecularGlossiness = 1, // Bistro(G3D 규약): metallicRoughnessTextureIndex 슬롯에 _spec 텍스처
                            //   RGB = F0 반사색(sRGB), A = smoothness(glossiness)
};

struct MaterialData
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
    alignas(4) cInt workflow = static_cast<cInt>(MaterialWorkflow::MetallicRoughness);
    alignas(4) cInt padding0 = 0; // 원래 있던 꼬리 패딩 자리. 크기(80)를 바꾸지 않는다.
};

// std430 배열 stride 와 같아야 한다 (셰이더의 MaterialUBO 배열).
static_assert(sizeof(MaterialData) == 80, "MaterialData must stay 80 bytes (std430 stride)");

#endif // !INCLUDE_MATERIAL_H_
