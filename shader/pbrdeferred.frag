#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

// G-Buffer 쓰기 (deferred 1단계). 조명은 lightdeferred.comp 가 한다.
// 버텍스 셰이더는 pbrdeferred.vert 를 그대로 쓴다.

#include "materialCommon.glsl"        // MAX_TEXTURE_COUNT, DUMMY_TEXTURE_INDEX, MATERIAL_WORKFLOW_*
#include "deferredShadingCommon.glsl" // SceneDataUBO, OptionsUBO, EncodeNormal, G-Buffer 배치

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragCameraPos;
layout(location = 6) in vec4 fragPosLightSpace;

// coeffs 사용처 (C++ DrawModelResource, 128 바이트):
//   [2] = 모델별 발광 오프셋, [4] / [5] = 모델별 금속성 / 거칠기 오프셋 (GUI)
//   [0] / [1] / [3] 은 쓰지 않는다.
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

layout(set = 1, binding = 0) restrict readonly buffer MaterialBuffer {
    MaterialUBO materials[];
} materialBuffer;

layout(set = 1, binding = 1) uniform sampler2D materialTextures[MAX_TEXTURE_COUNT];

layout(location = 0) out vec4 gAlbedo;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gPosition;
layout(location = 3) out vec4 gMaterial;

// 0 에 가까운 Roughness 는 GGX 하이라이트가 한 점으로 모여 깜빡이므로 하한을 둔다.
const float MIN_ROUGHNESS = 0.045;
const float OPACITY_DISCARD_THRESHOLD = 0.08;

bool HasTexture(int TextureIndex)
{
    return options.textureOn != 0 && TextureIndex > DUMMY_TEXTURE_INDEX;
}

vec4 SampleMaterialTexture(int TextureIndex)
{
    return texture(materialTextures[nonuniformEXT(TextureIndex)], fragTexCoord);
}

// ---------------------------------------------------------------------------
// Specular-Glossiness -> Metallic-Roughness 변환
// (KHR_materials_pbrSpecularGlossiness 변환기에서 쓰는 공식)
// 조명 패스는 metallic-roughness 만 알기 때문에 G-Buffer 에 쓰기 전에 여기서 바꾼다.
// ---------------------------------------------------------------------------
const float DIELECTRIC_F0 = 0.04;

float PerceivedBrightness(vec3 Color)
{
    return sqrt(0.299 * Color.r * Color.r + 0.587 * Color.g * Color.g + 0.114 * Color.b * Color.b);
}

float SolveMetallic(float Diffuse, float Specular, float OneMinusSpecularStrength)
{
    if (Specular < DIELECTRIC_F0)
        return 0.0;

    float a = DIELECTRIC_F0;
    float b = Diffuse * OneMinusSpecularStrength / (1.0 - DIELECTRIC_F0) + Specular - 2.0 * DIELECTRIC_F0;
    float c = DIELECTRIC_F0 - Specular;
    float D = max(b * b - 4.0 * a * c, 0.0);
    return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}

// Diffuse: 선형 diffuse 색, Specular: 선형 F0 색 -> (BaseColor, Metallic)
void ConvertSpecularGlossiness(vec3 Diffuse, vec3 Specular, out vec3 BaseColor, out float Metallic)
{
    float OneMinusSpecularStrength = 1.0 - max(max(Specular.r, Specular.g), Specular.b);
    Metallic = SolveMetallic(PerceivedBrightness(Diffuse), PerceivedBrightness(Specular), OneMinusSpecularStrength);

    vec3 BaseColorFromDiffuse = Diffuse * OneMinusSpecularStrength / (1.0 - DIELECTRIC_F0) / max(1.0 - Metallic, 1e-4);
    vec3 BaseColorFromSpecular = (Specular - DIELECTRIC_F0 * (1.0 - Metallic)) / max(Metallic, 1e-4);
    BaseColor = clamp(mix(BaseColorFromDiffuse, BaseColorFromSpecular, Metallic * Metallic), 0.0, 1.0);
}

void main()
{
    MaterialUBO Material = materialBuffer.materials[pushConstants.materialIndex];

    // ---- 알파 테스트 ----------------------------------------------------
    // (textureOn 과 무관하게 discardOn 만 따른다 — 원본 pbrdeferred.frag 와 같은 동작)
    if (options.discardOn != 0 && Material.opacityTextureIndex > DUMMY_TEXTURE_INDEX)
    {
        float Opacity = SampleMaterialTexture(Material.opacityTextureIndex).r;
        if (Opacity < OPACITY_DISCARD_THRESHOLD)
            discard;
    }

    // glTF alphaMode == MASK: 알파는 baseColorFactor.a * baseColor 텍스처 a 에 있고,
    // discardAlpha 에 alphaCutoff 가 들어 있다 (ModelLoader::processMaterial).
    if (options.discardOn != 0 && Material.alphaMode == MATERIAL_ALPHA_MODE_MASK)
    {
        float Alpha = Material.baseColorFactor.a;
        if (Material.baseColorTextureIndex > DUMMY_TEXTURE_INDEX)
            Alpha *= SampleMaterialTexture(Material.baseColorTextureIndex).a;
        if (Alpha < Material.discardAlpha)
            discard;
    }

    // ---- BaseColor / Metallic / Roughness -------------------------------
    vec3 BaseColor = Material.baseColorFactor.rgb;
    if (HasTexture(Material.baseColorTextureIndex))
    {
        BaseColor *= SampleMaterialTexture(Material.baseColorTextureIndex).rgb;
    }

    float Metallic = Material.metallicFactor;
    float Roughness = Material.roughnessFactor;

    if (HasTexture(Material.metallicRoughnessTextureIndex))
    {
        vec4 MaterialSample = SampleMaterialTexture(Material.metallicRoughnessTextureIndex);

        if (Material.workflow == MATERIAL_WORKFLOW_SPECULAR_GLOSSINESS)
        {
            // Bistro _spec.png: sRGB 포맷이라 rgb 는 이미 선형 F0, a 는 선형 smoothness
            ConvertSpecularGlossiness(BaseColor, MaterialSample.rgb, BaseColor, Metallic);

            // 알파가 없는 PNG 는 a 가 1.0 으로 읽힌다. 이걸 smoothness 1 로 믿으면 거울이 되므로
            // 이때는 MTL 의 Ns 에서 구한 roughnessFactor 를 쓴다.
            Roughness = (MaterialSample.a > 0.999) ? Material.roughnessFactor : 1.0 - MaterialSample.a;
        }
        else
        {
            Metallic *= MaterialSample.b;  // glTF: B = metallic
            Roughness *= MaterialSample.g; // glTF: G = roughness
        }
    }

    // GUI 디버그 오프셋. 기본값 0 = 저작된 머티리얼 값 그대로.
    // 곱셈이 아니라 덧셈인 이유: metallicFactor 가 0 인 머티리얼에서는 곱셈으로는 아무 효과가 없다.
    Metallic += pushConstants.coeffs[4];
    Roughness += pushConstants.coeffs[5];

    Metallic = clamp(Metallic, 0.0, 1.0);
    Roughness = clamp(Roughness, MIN_ROUGHNESS, 1.0);

    // ---- AO / Emissive ---------------------------------------------------
    float AmbientOcclusion = 1.0;
    if (HasTexture(Material.occlusionTextureIndex))
    {
        AmbientOcclusion = SampleMaterialTexture(Material.occlusionTextureIndex).r;
    }

    // 모델별 GUI 값(coeffs[2])은 덧셈 오프셋이다. 기본 0 = 저작된 값 그대로.
    vec3 Emissive = Material.emissiveFactor.rgb * (1.0 + pushConstants.coeffs[2]);
    if (HasTexture(Material.emissiveTextureIndex))
    {
        Emissive *= SampleMaterialTexture(Material.emissiveTextureIndex).rgb;
    }

    // ---- 노멀 (노멀맵) ----------------------------------------------------
    vec3 N = normalize(fragNormal);
    if (HasTexture(Material.normalTextureIndex))
    {
        vec3 T = normalize(fragTangent);
        vec3 B = normalize(fragBitangent);
        vec3 TangentNormal = SampleMaterialTexture(Material.normalTextureIndex).xyz * 2.0 - 1.0;
        if (length(TangentNormal) > 0.5)
        {
            N = normalize(mat3(T, B, N) * TangentNormal);
        }
    }

    // ---- G-Buffer (배치는 deferredShadingCommon.glsl) ---------------------
    gAlbedo = vec4(BaseColor, Metallic);
    gNormal = vec4(EncodeNormal(N), Roughness);
    gPosition = vec4(fragPos, 0.0);
    gMaterial = vec4(Emissive, AmbientOcclusion);
}
