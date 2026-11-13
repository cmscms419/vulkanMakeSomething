#version 450

// 천 프래그먼트 셰이더 — G-Buffer 에 덧그린다.
//
// pbrdeferred 가 G-Buffer 를 채운 뒤에 LOAD 로 이어서 그리므로,
// 조명/그림자/SSAO 는 lightdeferred 패스가 알아서 해준다.
// 출력 4장의 의미는 pbrdeferred.frag / physicsInstance.frag 와 똑같아야 한다.

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

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal; // cloth.vert 가 격자 이웃으로 만든 정점 법선 (보간됨)

layout(location = 0) out vec4 gAlbedo;   // RGB: albedo,          A: metallic
layout(location = 1) out vec4 gNormal;   // RGB: world normal,    A: roughness
layout(location = 2) out vec4 gPosition; // RGB: world position,  A: depth
layout(location = 3) out vec4 gMaterial; // RGB: emissive,        A: AO

// 천 머티리얼. 지금은 상수다 — ImGui 로 조절하는 것은 Step 8 에서 한다.
const vec3 kClothColor = vec3(0.72, 0.20, 0.24);
const float kClothRoughness = 0.85;
const float kClothMetallic = 0.0;

vec3 encodeNormal(vec3 normal) {
    return normal * 0.5 + 0.5;
}

void main() {
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

    gAlbedo = vec4(kClothColor, kClothMetallic);
    gNormal = vec4(encodeNormal(N), kClothRoughness);
    gPosition = vec4(fragPos, gl_FragCoord.z);
    gMaterial = vec4(vec3(0.0), 1.0);
}
