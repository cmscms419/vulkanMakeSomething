#ifndef DEFERRED_SHADING_COMMON_GLSL
#define DEFERRED_SHADING_COMMON_GLSL

// 이름은 Unreal Engine 의 DeferredShadingCommon.ush 를 따른다.
// G-Buffer 배치 (두 셰이더가 반드시 같은 규칙을 써야 한다)
//   gAlbedo   : RGB = BaseColor,                 A = Metallic
//   gNormal   : RGB = EncodeNormal(월드 노멀),   A = Roughness
//   gPosition : RGB = 월드 위치,                 A = 쓰지 않음 (0)
//   gMaterial : RGB = Emissive,                  A = AO (머티리얼 텍스처)

layout(set = 0, binding = 0) uniform SceneDataUBO {
    mat4 projection;
    mat4 view;
    vec3 cameraPos;
    float padding1;
    vec3 directionalLightDir;   // 표면 -> 빛 방향
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
    float specularWeight; // 쓰지 않음
    float diffuseWeight;  // 쓰지 않음
    float emissiveWeight; // 쓰지 않음
    float shadowOffset;   // 쓰지 않음
} options;

// [-1, 1] 노멀을 [0, 1] 로 저장
vec3 EncodeNormal(vec3 N)
{
    return N * 0.5 + 0.5;
}

vec3 DecodeNormal(vec3 N)
{
    return N * 2.0 - 1.0;
}

#endif // DEFERRED_SHADING_COMMON_GLSL
