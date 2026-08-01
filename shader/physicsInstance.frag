#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

// draw call당 1번만 전달 — 공유 sphere 메쉬는 머티리얼이 하나뿐이므로 인스턴스마다 다르지 않음
layout(push_constant) uniform PushConstants {
    uint materialIndex;
} pc;

// pbrdeferred.frag와 동일한 MaterialUBO/바인드리스 텍스처 배열 재사용 (set=1)
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
};

layout(set = 1, binding = 0) restrict readonly buffer MaterialBuffer {
    MaterialUBO materials[];
} materialBuffer;

layout(set = 1, binding = 1) uniform sampler2D materialTextures[512];

// pbrdeferred와 같은 G-Buffer 출력 — lightdeferred 패스가 이 값을 그대로 읽어서 조명 계산
layout(location = 0) out vec4 gAlbedo;      // RGB: albedo, A: metallic
layout(location = 1) out vec4 gNormal;      // RGB: world-space normal(encoded), A: roughness
layout(location = 2) out vec4 gPosition;    // RGB: world-space position, A: depth
layout(location = 3) out vec4 gMaterial;    // RGB: emissive, A: AO

vec3 encodeNormal(vec3 normal) {
    return normal * 0.5 + 0.5;
}

void main() {
    MaterialUBO material = materialBuffer.materials[pc.materialIndex];

    vec4 baseColorRGBA = material.baseColorTextureIndex >= 0
        ? texture(materialTextures[nonuniformEXT(material.baseColorTextureIndex)], fragTexCoord)
        : vec4(1.0);

    vec3 baseColor = material.baseColorFactor.rgb * baseColorRGBA.rgb;
    float metallic = clamp(material.metallicFactor, 0.0, 1.0);
    float roughness = clamp(material.roughnessFactor, 0.0, 1.0);

    vec3 N = normalize(fragNormal);

    gAlbedo = vec4(baseColor, metallic);
    gNormal = vec4(encodeNormal(N), roughness);
    gPosition = vec4(fragPos, gl_FragCoord.z);
    gMaterial = vec4(material.emissiveFactor.rgb, 1.0); // AO 텍스처 미사용 → 1.0 고정
}
