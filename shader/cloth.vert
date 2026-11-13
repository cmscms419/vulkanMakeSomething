#version 450

// 천 정점 셰이더.
//
// 정점 버퍼는 clothSolve.comp 가 매 프레임 고쳐 쓰는 그 버퍼다 (ClothVertex, 64 byte).
// 맨 앞 vec3 가 position 이라 그것만 attribute 로 받는다.
//
// 법선: ClothVertex 에 법선 칸이 없으므로, 같은 버퍼를 SSBO(set 1)로도 묶어
// 격자 이웃 4개의 위치로 정점 법선을 만든다 (중앙 차분).
// 프레임 끝 배리어(ClothDataManagement::insertFrameEndBarrier)가
// VERTEX_SHADER + SHADER_STORAGE_READ 까지 막아 주므로 추가 동기화가 필요 없다.
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

// ClothVertex(64 byte) 를 vec4 4개로 본다. 위치만 쓴다.
struct ClothVertexData {
    vec4 positionMass;       // xyz = position, w = mass
    vec4 prevPositionPinned;
    vec4 nextPosition;
    vec4 velocity;
};

layout(std430, set = 1, binding = 0) readonly buffer ClothVertexBuffer {
    ClothVertexData vertices[];
} clothVertices;

// Renderer::makeClothPass 가 넣는다. 정점 번호 = y * gridX + x (ClothSimulation::getVertexIndex)
layout(push_constant) uniform ClothDrawConstants {
    uint gridX;
    uint gridY;
} drawConstants;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;

vec3 loadPosition(uint x, uint y) {
    return clothVertices.vertices[y * drawConstants.gridX + x].positionMass.xyz;
}

void main() {
    uint index = uint(gl_VertexIndex);
    uint x = index % drawConstants.gridX;
    uint y = index / drawConstants.gridX;

    // 가장자리에서는 한쪽 차분이 되도록 이웃 번호를 격자 안으로 자른다.
    uint xMinus = (x > 0u) ? x - 1u : x;
    uint xPlus  = min(x + 1u, drawConstants.gridX - 1u);
    uint yMinus = (y > 0u) ? y - 1u : y;
    uint yPlus  = min(y + 1u, drawConstants.gridY - 1u);

    vec3 tangentX = loadPosition(xPlus, y) - loadPosition(xMinus, y);
    vec3 tangentY = loadPosition(x, yPlus) - loadPosition(x, yMinus);
    vec3 normal = cross(tangentX, tangentY);

    // 천이 한 점으로 뭉개진 경우 normalize 가 NaN 을 만들지 않게 한다.
    float normalLength = length(normal);
    fragNormal = (normalLength > 1e-8) ? normal / normalLength : vec3(0.0, 0.0, 1.0);

    fragPos = inPosition;
    gl_Position = sceneData.projection * sceneData.view * vec4(inPosition, 1.0);
}
