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

// 알파 처리 방식 (glTF alphaMode).
// shader/materialCommon.glsl 의 MATERIAL_ALPHA_MODE_* define 과 값이 반드시 일치해야 한다.
enum class MaterialAlphaMode : cInt
{
    Opaque = 0, // 알파 무시
    Mask = 1,   // baseColorFactor.a * baseColor 텍스처 a < discardAlpha(alphaCutoff) 이면 discard
    Blend = 2,  // 반투명 (아직 별도 처리 없음, Opaque 처럼 그려진다)
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
    alignas(4) cInt alphaMode = static_cast<cInt>(MaterialAlphaMode::Opaque);
    alignas(16) cVec4 clothFactor = cVec4(0.0f);
};

// std430 배열 stride 와 같아야 한다 (셰이더의 MaterialUBO 배열).
static_assert(sizeof(MaterialData) == 96, "MaterialData must be 96 bytes (std430 stride)");

#endif // !INCLUDE_MATERIAL_H_
