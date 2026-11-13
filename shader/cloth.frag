#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#include "materialCommon.glsl"

// 천 프래그먼트 셰이더 — G-Buffer 에 덧그린다.
//
// pbrdeferred 가 G-Buffer 를 채운 뒤에 LOAD 로 이어서 그리므로,
// 조명/그림자/SSAO 는 lightdeferred 패스가 알아서 해준다.
// 출력 4장의 의미는 pbrdeferred.frag / physicsInstance.frag 와 똑같아야 한다.
//
// 머티리얼은 엔진의 전역 머티리얼 버퍼(set 2)에서 읽는다.
// Renderer 가 예약한 칸(clothMaterialIndex)을 데모가 setClothMaterial() 로 채운다.
// 셰이딩 모델은 SHADING_MODEL_CLOTH 로 기록한다 — lightdeferred 가 sheen/투과로 조명한다.

layout(std140, set = 0, binding = 0) uniform SceneDataUBO {
    mat4 projection;
    mat4 view;
    vec3 cameraPos;
    float padding1;
    vec3 directionalLightDir;
    float padding2;
    vec3 directionalLightColor;
    float padding3;
    mat4 lightSpaceMatrix;
} sceneData;

// pbrdeferred.frag 의 MaterialUBO, C++ MaterialData(80 byte)와 같은 배치
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
    int padding0;
    vec4 clothFactor; // x = sheen 세기, y = 얇은 천 투과
};

layout(set = 2, binding = 0) restrict readonly buffer MaterialBuffer {
    MaterialUBO materials[];
} materialBuffer;

layout(set = 2, binding = 1) uniform sampler2D materialTextures[MAX_TEXTURE_COUNT];

// cloth.vert 와 블록 선언이 같아야 한다.
layout(push_constant) uniform ClothDrawConstants {
    uint gridX;
    uint gridY;
    uint materialIndex;
} drawConstants;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;   // cloth.vert 가 격자 이웃으로 만든 정점 법선 (보간됨)
layout(location = 2) in vec2 fragTexCoord; // 격자 UV
layout(location = 3) in vec3 fragTangent;  // u 방향

layout(location = 0) out vec4 gAlbedo;   // RGB: albedo,          A: metallic
layout(location = 1) out vec4 gNormal;   // RGB: world normal,    A: roughness
layout(location = 2) out vec4 gPosition; // RGB: world position,  A: depth
layout(location = 3) out vec4 gMaterial; // RGB: emissive,        A: AO

vec3 encodeNormal(vec3 normal) {
    return normal * 0.5 + 0.5;
}

bool hasTexture(int index) {
    return index > DUMMY_TEXTURE_INDEX;
}

vec4 sampleMaterialTexture(int index) {
    return texture(materialTextures[nonuniformEXT(index)], fragTexCoord);
}

void main() {
    MaterialUBO material = materialBuffer.materials[drawConstants.materialIndex];

    // ---- 법선 ---------------------------------------------------------
    // 정점 법선(보간) — 면 경계가 보이지 않는 부드러운 음영.
    vec3 N = normalize(fragNormal);

    // 천은 양면이다 (cullMode = NONE). 보이는 쪽 면으로 법선을 맞춰야 뒷면이 검게 보이지 않는다.
    // 기준은 화면 미분으로 만든 "면 법선을 카메라 쪽으로 돌린 것" 이다.
    //  - 면 법선은 삼각형 안에서 일정하므로, 픽셀마다 시선 방향으로 뒤집을 때처럼
    //    한 삼각형 안에서 앞/뒤가 갈리는 일이 없다.
    //  - 삼각형 감기 방향이나 Vulkan 의 y 뒤집기에 기대지 않아도 된다 (gl_FrontFacing 대신).
    vec3 faceNormal = normalize(cross(dFdx(fragPos), dFdy(fragPos)));
    if (dot(faceNormal, sceneData.cameraPos - fragPos) < 0.0) {
        faceNormal = -faceNormal;
    }
    if (dot(N, faceNormal) < 0.0) {
        N = -N;
    }

    // 노멀맵 (직조 요철). 뒷면에서는 N 이 뒤집혔으므로 B = cross(N, T) 도 따라 뒤집힌다.
    if (hasTexture(material.normalTextureIndex)) {
        vec3 T = normalize(fragTangent - N * dot(N, fragTangent)); // N 에 직교화
        vec3 B = cross(N, T);
        vec3 tangentNormal = sampleMaterialTexture(material.normalTextureIndex).xyz * 2.0 - 1.0;
        N = normalize(mat3(T, B, N) * tangentNormal);
    }

    // ---- 머티리얼 -----------------------------------------------------
    vec3 baseColor = material.baseColorFactor.rgb;
    if (hasTexture(material.baseColorTextureIndex)) {
        baseColor *= sampleMaterialTexture(material.baseColorTextureIndex).rgb;
    }

    // 천 셰이딩 모델은 금속성을 쓰지 않는다 (천은 유전체). MR 텍스처는 거칠기(G)만 읽는다.
    // 거칠기는 sheen 의 거칠기로도 쓰인다 (lightdeferred 의 Charlie 분포).
    float roughness = material.roughnessFactor;
    if (hasTexture(material.metallicRoughnessTextureIndex)) {
        roughness *= sampleMaterialTexture(material.metallicRoughnessTextureIndex).g;
    }

    float sheenIntensity = clamp(material.clothFactor.x, 0.0, 1.0);
    float translucency = clamp(material.clothFactor.y, 0.0, 1.0);

    float ao = 1.0;
    if (hasTexture(material.occlusionTextureIndex)) {
        ao = sampleMaterialTexture(material.occlusionTextureIndex).r;
    }

    vec3 emissive = material.emissiveFactor.rgb;
    if (hasTexture(material.emissiveTextureIndex)) {
        emissive *= sampleMaterialTexture(material.emissiveTextureIndex).rgb;
    }

    // pbrdeferred.frag 와 같은 하한 (GGX 하이라이트가 한 점으로 모여 깜빡이는 것 방지)
    roughness = clamp(roughness, 0.045, 1.0);

    // 천 셰이딩 모델의 G-buffer 해석 (materialCommon.glsl 참고)
    //   gAlbedo.a   = sheen 세기 (STANDARD 에서는 metallic 자리)
    //   gPosition.a = CLOTH + 얇은 천 투과
    gAlbedo = vec4(baseColor, sheenIntensity);
    gNormal = vec4(encodeNormal(N), roughness);
    gPosition = vec4(fragPos, packShadingModel(SHADING_MODEL_CLOTH, translucency));
    gMaterial = vec4(emissive, ao);
}
