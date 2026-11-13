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
    alignas(4) cInt padding0 = 0;

    // 천 셰이딩 모델 전용 (cloth.frag 만 읽는다). 다른 머티리얼은 0 으로 둔다.
    //   x = sheen 세기 (0~1)   — sheen 색 = x * sqrt(baseColor)   (Filament cloth 기본값 방식)
    //   y = 얇은 천 투과 (0~1) — 역광일 때 뒤에서 비쳐 들어오는 빛의 비율
    //   z, w = 예약
    alignas(16) cVec4 clothFactor = cVec4(0.0f);
};

// std430 배열 stride 와 같아야 한다 (셰이더의 MaterialUBO 배열).
// 이 구조체를 선언한 셰이더: pbrdeferred.frag, pbrForward.frag, physicsInstance.frag, cloth.frag
static_assert(sizeof(MaterialData) == 96, "MaterialData must be 96 bytes (std430 stride)");

#endif // !INCLUDE_MATERIAL_H_
