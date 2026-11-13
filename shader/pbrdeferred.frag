#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#include "materialCommon.glsl"

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragCameraPos;
layout(location = 6) in vec4 fragPosLightSpace;

layout(push_constant) uniform PushConstants {
    mat4 model;
    float coeffs[15];
    uint materialIndex;
} pushConstants;

layout(set = 0, binding = 0) uniform SceneDataUBO {
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

// Material structure matching MaterialUBO in C++
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
    int metallicRoughnessTextureIndex; // workflow == SPECULAR_GLOSSINESS 이면 _spec 텍스처(RGB=F0, A=smoothness)
    int occlusionTextureIndex;
    int workflow;                      // MATERIAL_WORKFLOW_* (materialCommon.glsl)
    int padding0;
};

// Bindless material storage buffer
layout(set = 1, binding = 0) restrict readonly buffer MaterialBuffer {
    MaterialUBO materials[];
} materialBuffer;

// Material textures
layout(set = 1, binding = 1) uniform sampler2D materialTextures[MAX_TEXTURE_COUNT]; 
// 512 is TextureManager::kMaxTextures

// G-Buffer outputs for deferred rendering
layout(location = 0) out vec4 gAlbedo;      // RGB: albedo/baseColor, A: metallic
layout(location = 1) out vec4 gNormal;      // RGB: world-space normal (encoded), A: roughness
layout(location = 2) out vec4 gPosition;    // RGB: world-space position, A: depth
layout(location = 3) out vec4 gMaterial;    // RGB: emissive color, A: AO

// Normal encoding function for G-buffer
vec3 encodeNormal(vec3 normal) {
    return normal * 0.5 + 0.5;
}

// ---------------------------------------------------------------------------
// Specular-Glossiness -> Metallic-Roughness 변환
// (KHR_materials_pbrSpecularGlossiness 변환기에서 쓰는 공식)
// lighting 패스는 metallic-roughness 만 알기 때문에 G-buffer 에 쓰기 전에 여기서 바꾼다.
// ---------------------------------------------------------------------------
const float DIELECTRIC_F0 = 0.04;

float perceivedBrightness(vec3 c) {
    return sqrt(0.299 * c.r * c.r + 0.587 * c.g * c.g + 0.114 * c.b * c.b);
}

float solveMetallic(float diffuse, float specular, float oneMinusSpecularStrength) {
    if (specular < DIELECTRIC_F0)
        return 0.0;

    float a = DIELECTRIC_F0;
    float b = diffuse * oneMinusSpecularStrength / (1.0 - DIELECTRIC_F0) + specular - 2.0 * DIELECTRIC_F0;
    float c = DIELECTRIC_F0 - specular;
    float D = max(b * b - 4.0 * a * c, 0.0);
    return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}

// diffuse: 선형 diffuse 색, specular: 선형 F0 색 -> (baseColor, metallic)
void convertSpecularGlossiness(vec3 diffuse, vec3 specular, out vec3 baseColor, out float metallic) {
    float oneMinusSpecularStrength = 1.0 - max(max(specular.r, specular.g), specular.b);
    metallic = solveMetallic(perceivedBrightness(diffuse), perceivedBrightness(specular), oneMinusSpecularStrength);

    vec3 baseColorFromDiffuse = diffuse * oneMinusSpecularStrength / (1.0 - DIELECTRIC_F0) / max(1.0 - metallic, 1e-4);
    vec3 baseColorFromSpecular = (specular - DIELECTRIC_F0 * (1.0 - metallic)) / max(metallic, 1e-4);
    baseColor = clamp(mix(baseColorFromDiffuse, baseColorFromSpecular, metallic * metallic), 0.0, 1.0);
}

void main() {
    // coeffs 사용처 (C++ DrawModelResource 와 레이아웃은 그대로 128 바이트):
    //   [2] = 모델별 발광 오프셋, [4]/[5] = 모델별 금속성/거칠기 오프셋.
    //   [0]/[1]/[3] 은 forward 시절 반사광/난반사/그림자 오프셋 자리였는데 deferred 에서는 쓰지 않는다.
    float emissiveWeight = pushConstants.coeffs[2];

    // Access material using push constant index
    MaterialUBO material = materialBuffer.materials[pushConstants.materialIndex];

    // Sample material properties using bindless access
    vec4 baseColorRGBA = (options.textureOn != 0 && material.baseColorTextureIndex > DUMMY_TEXTURE_INDEX) ? 
                        texture(materialTextures[nonuniformEXT(material.baseColorTextureIndex)], fragTexCoord) : 
                        vec4(1.0);

    if(options.discardOn != 0 && material.opacityTextureIndex > DUMMY_TEXTURE_INDEX)
    {
        float opacity = texture(materialTextures[nonuniformEXT(material.opacityTextureIndex)], fragTexCoord).r;
        if(opacity < 0.08)
            discard;
    }

    vec3 baseColor = material.baseColorFactor.rgb * baseColorRGBA.rgb;
    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;

    bool hasMaterialTexture = options.textureOn != 0 && material.metallicRoughnessTextureIndex > DUMMY_TEXTURE_INDEX;

    if (hasMaterialTexture && material.workflow == MATERIAL_WORKFLOW_SPECULAR_GLOSSINESS) {
        // Bistro _spec.png: sRGB 포맷이라 rgb 는 이미 선형 F0, a 는 선형 smoothness
        vec4 specularGlossiness = texture(materialTextures[nonuniformEXT(material.metallicRoughnessTextureIndex)], fragTexCoord);
        convertSpecularGlossiness(baseColor, specularGlossiness.rgb, baseColor, metallic);

        // 알파가 없는 PNG(L/RGB, 팔레트에 tRNS 없음)는 a 가 1.0 으로 읽힌다.
        // 이걸 smoothness 1 로 믿으면 거울이 되므로, 이때는 MTL 의 Ns 에서 구한 roughness 를 쓴다.
        roughness = (specularGlossiness.a > 0.999) ? material.roughnessFactor : 1.0 - specularGlossiness.a;
    }
    else if (hasMaterialTexture) {
        vec3 metallicRoughness = texture(materialTextures[nonuniformEXT(material.metallicRoughnessTextureIndex)], fragTexCoord).rgb;
        metallic *= metallicRoughness.b; // Blue channel
        roughness *= metallicRoughness.g; // Green channel
    }

    // GUI 디버그 오프셋. coeffs[4]/[5] 기본값 0 = 저작된 머티리얼 값 그대로.
    // 곱셈이 아니라 덧셈인 이유: metallicFactor 가 0 인 머티리얼(예: sponza 전 머티리얼)에서는
    // 곱셈으로는 슬라이더가 영원히 0 에 묶여 아무 효과가 없다.
    metallic += pushConstants.coeffs[4];
    roughness += pushConstants.coeffs[5];

    float ao = 1.0;
    if(options.textureOn != 0 && material.occlusionTextureIndex > DUMMY_TEXTURE_INDEX){
        ao = texture(materialTextures[nonuniformEXT(material.occlusionTextureIndex)], fragTexCoord).r;
    }

    // emissive 전역 세기(options.emissiveWeight)는 lightdeferred 에서 한 번만 곱한다.
    // 모델별 GUI 값(coeffs[2])은 metallic/roughness 처럼 덧셈 오프셋이다. 기본 0 = 저작된 값 그대로.
    vec3 emissive = material.emissiveFactor.xyz * (1.0 + emissiveWeight);
    if(options.textureOn != 0 && material.emissiveTextureIndex > DUMMY_TEXTURE_INDEX){
        emissive *= texture(materialTextures[nonuniformEXT(material.emissiveTextureIndex)], fragTexCoord).rgb;
    }

    // Calculate world-space normal with normal mapping
    vec3 N = normalize(fragNormal);
    if(options.textureOn != 0 && material.normalTextureIndex > DUMMY_TEXTURE_INDEX) {
        vec3 T = normalize(fragTangent);
        vec3 B = normalize(fragBitangent);
        mat3 TBN = mat3(T, B, N);
        
        vec3 tangentNormal = texture(materialTextures[nonuniformEXT(material.normalTextureIndex)], fragTexCoord).xyz * 2.0 - 1.0;
        if (length(tangentNormal) > 0.5)
            N = normalize(TBN * tangentNormal);
    }

    // Output to G-Buffer
    // Clamp material properties
    // 0 에 가까운 roughness 는 GGX 하이라이트가 한 점으로 모여 깜빡이므로 하한을 둔다.
    roughness = clamp(roughness, 0.045, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);
    
    gAlbedo = vec4(baseColor, metallic);
    gNormal = vec4(encodeNormal(N), roughness);
    gPosition = vec4(fragPos, gl_FragCoord.z);
    gMaterial = vec4(emissive, ao);
}
