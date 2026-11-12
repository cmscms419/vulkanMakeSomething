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
    // 법선을 화면 미분으로 만든다.
    // ClothVertex 64 byte 에 법선 칸이 없어서(position/prevPosition/nextPosition/velocity)
    // 정점에서 들고 올 수가 없다. 이 방법은 면 단위라 각져 보인다.
    // 매끄럽게 하려면 정점 셰이더에서 SSBO 로 이웃을 읽어야 한다.
    vec3 N = normalize(cross(dFdx(fragPos), dFdy(fragPos)));

    // 천은 양면이다 (cullMode = NONE).
    // 뒷면에서는 위 법선이 반대로 나오므로 카메라 쪽으로 뒤집는다.
    // 안 하면 천의 절반이 검게 보인다.
    if (dot(N, sceneData.cameraPos - fragPos) < 0.0) {
        N = -N;
    }

    gAlbedo = vec4(kClothColor, kClothMetallic);
    gNormal = vec4(encodeNormal(N), kClothRoughness);
    gPosition = vec4(fragPos, gl_FragCoord.z);
    gMaterial = vec4(vec3(0.0), 1.0);
}
