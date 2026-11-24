#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

// Fragment shader for shadow map generation.
// 깊이는 고정 기능이 쓰므로 여기서는 알파 테스트만 한다.
// 판정 규칙은 pbrdeferred.frag 와 같아야 한다 — 다르면 보이는 모양과 그림자 모양이 어긋난다.
//   - opacity 텍스처(Bistro map_d): .r < OPACITY_DISCARD_THRESHOLD 이면 discard
//   - glTF alphaMode == MASK     : baseColorFactor.a * baseColor.a < discardAlpha(alphaCutoff) 이면 discard

#include "materialCommon.glsl" // MAX_TEXTURE_COUNT, DUMMY_TEXTURE_INDEX, MATERIAL_ALPHA_MODE_*

layout(location = 0) in vec2 fragTexCoord;

// shadowMap.vert 의 OptionsUBO 와 같은 바인딩 (bool 과 int 는 std140 에서 둘 다 4 바이트)
layout(set = 0, binding = 1) uniform OptionsUBO {
    int textureOn;
    int shadowOn;
    int discardOn;
    int animationOn;
    float specularWeight;
    float diffuseWeight;
    float emissiveWeight;
    float shadowOffset;
} options;

// shadowMap.vert 와 블록 선언이 같아야 한다 (C++ DrawModelResource, 128 바이트).
layout(push_constant) uniform PushConstants {
    mat4 model;
    float coeffs[15];
    uint materialIndex;
} pushConstants;

// C++ MaterialData (include/struct/materialData.h, 96 바이트) 와 같은 배치
struct MaterialUBO {
    vec4 emissiveFactor;
    vec4 baseColorFactor;
    float roughnessFactor;
    float transparencyFactor;
    float discardAlpha;
    float metallicFactor;
    int baseColorTextureIndex;
    int emissiveTextureIndex;
    int normalTextureIndex;
    int opacityTextureIndex;
    int metallicRoughnessTextureIndex;
    int occlusionTextureIndex;
    int workflow;
    int alphaMode;                     // MATERIAL_ALPHA_MODE_* (materialCommon.glsl)
    vec4 clothFactor;
};

// pbrdeferred 의 set 1 과 같은 레이아웃 — Renderer 의 materialDescriptorSet 을 그대로 바인딩한다.
layout(set = 1, binding = 0) restrict readonly buffer MaterialBuffer {
    MaterialUBO materials[];
} materialBuffer;

layout(set = 1, binding = 1) uniform sampler2D materialTextures[MAX_TEXTURE_COUNT];

// pbrdeferred.frag 의 같은 이름 상수와 값이 같아야 한다.
const float OPACITY_DISCARD_THRESHOLD = 0.08;

void main()
{
    if (options.discardOn == 0)
        return;

    MaterialUBO Material = materialBuffer.materials[pushConstants.materialIndex];

    if (Material.opacityTextureIndex > DUMMY_TEXTURE_INDEX)
    {
        float Opacity = texture(materialTextures[nonuniformEXT(Material.opacityTextureIndex)], fragTexCoord).r;
        if (Opacity < OPACITY_DISCARD_THRESHOLD)
            discard;
    }

    if (Material.alphaMode == MATERIAL_ALPHA_MODE_MASK)
    {
        float Alpha = Material.baseColorFactor.a;
        if (Material.baseColorTextureIndex > DUMMY_TEXTURE_INDEX)
            Alpha *= texture(materialTextures[nonuniformEXT(Material.baseColorTextureIndex)], fragTexCoord).a;
        if (Alpha < Material.discardAlpha)
            discard;
    }
}
