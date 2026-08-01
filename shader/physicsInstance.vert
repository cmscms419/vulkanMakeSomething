#version 450

// pbrdeferred.vert와 같은 SceneDataUBO 레이아웃 재사용 (set=0, binding=0)
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

// 인스턴스별 모델 행렬 — push constant/per-object UBO 대신 SSBO + gl_InstanceIndex로 조회
struct InstanceData {
    mat4 modelMatrix;
};

layout(set = 0, binding = 1) readonly buffer InstanceBuffer {
    InstanceData instances[];
} instanceBuffer;

// 메쉬 정점 (Vertex2, binding=0) — pbrdeferred.vert와 동일한 location, 본 데이터만 제외
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inBoneWeights;
layout(location = 6) in ivec4 inBoneIndices;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

void main() {
    mat4 model = instanceBuffer.instances[gl_InstanceIndex].modelMatrix;

    vec4 worldPos = model * vec4(inPosition, 1.0);
    fragPos = worldPos.xyz;

    // pbrdeferred.vert와 동일한 노멀 변환(비균일 스케일 대비 역전치 행렬)
    // mat3 normalMatrix = transpose(inverse(mat3(model)));
    // fragNormal = normalMatrix * inNormal;
    fragNormal = inNormal;

    fragTexCoord = inTexCoord;

    gl_Position = sceneData.projection * sceneData.view * worldPos;
}
