#version 450

// 천 정점 셰이더.
//
// 정점 버퍼는 clothSolve.comp 가 매 프레임 고쳐 쓰는 그 버퍼다 (ClothVertex, 64 byte).
// 맨 앞 vec3 가 position 이라 그것만 attribute 로 받는다.
// 뒤쪽 prevPosition / nextPosition / velocity 는 compute 전용이라 읽지 않는다.
//
// 모델 행렬이 없다. 시뮬레이션이 처음부터 월드 좌표로 풀기 때문이다.

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

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragPos;

void main() {
    fragPos = inPosition;
    gl_Position = sceneData.projection * sceneData.view * vec4(inPosition, 1.0);
}
