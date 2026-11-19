struct clothVertex {
	vec3 position; 		float mass;
	vec3 prevPosition;	float pinned;
	vec3 nextPosition;	float grabWeight;	// 마우스 구에 잡힌 정도 (0..1). 클릭한 프레임에 clothSolve 가 정한다
	vec3 velocity;		float padding2;
};

struct spring {
	uint vertexA;
	uint vertexB;
	float restLength; // spring length
	float stiffness; // spring stiffness
};

struct clothVertexAdjacency {
	uint offset;
	uint count;
};

layout(set = 0, binding = 0) uniform simulationParams {
	vec3 gravity;
	float h;
};

layout(std430, set = 0, binding = 1) buffer clothVertexBuffer {
	clothVertex vertices[];
};
layout(std430, set = 0, binding = 2) readonly buffer springBuffer {
	spring springs[];
};
layout(std430, set = 0, binding = 3) readonly buffer clothVertexAdjacencyBuffer {
	clothVertexAdjacency adjacency[];
};
layout(std430, set = 0, binding = 4) readonly buffer clothVertexAdjacencySpringBuffer {
	uint adjacencySprings[];
};
layout(std430, set = 0, binding = 5) readonly buffer ClothGroupBuffer  {
	uint groupVertices[];   // 동시 갱신 가능한 그룹별로 정렬된 정점 번호
};

// ---------------------------------------------------------------------
// 마우스로 잡기 — 구 중심으로 끌어당기기
//
// 클릭하면 마우스 레이 위에 반경 R 의 구를 놓는다. 구의 깊이는
// "레이에서 R 안에 있는 정점 중 카메라에 가장 가까운 것" 의 깊이다.
//   clothPredict : 클릭한 프레임에 후보 정점마다 atomicMin(grabDepthBits, 깊이)
//   clothSolve   : 클릭한 프레임 반복 0 에서 각 정점의 grabWeight 를 정하고 (capture),
//                  잡는 동안 E = ½ k w |x - c|² 를 더한다
// 깊이는 클릭 순간 평면(planeOrigin, planeNormal) 기준 거리라 항상 양수다.
// 양수 float 는 비트를 uint 로 봐도 크기 순서가 같으므로 uint atomicMin 으로 충분하다.
// ---------------------------------------------------------------------
#define GRAB_DEPTH_NONE 0xFFFFFFFFu   // 레이 근처에 정점이 없었다 = 허공을 눌렀다

layout(std430, set = 0, binding = 6) buffer ClothGrabBuffer {
	uint grabDepthBits;
};

// 클릭 순간 평면 dot(p - planeOrigin, planeNormal) = depth 와 지금 레이의 교점 = 구 중심.
// 레이가 평면과 거의 평행하거나 평면이 레이 뒤에 있으면 false (그 프레임은 당기지 않는다).
bool makeGrabCenter(vec3 rayOrigin, vec3 rayDirection, vec3 planeOrigin, vec3 planeNormal,
                    float depth, out vec3 outCenter)
{
	outCenter = vec3(0.0);

	float denominator = dot(rayDirection, planeNormal);
	if (denominator < 1e-4) {
		return false;
	}

	float t = (depth - dot(rayOrigin - planeOrigin, planeNormal)) / denominator;
	if (t <= 0.0) {
		return false;
	}

	outCenter = rayOrigin + rayDirection * t;
	return true;
}

// 구 안에서의 무게. 중심에서 1, 경계에서 0 이고 경계에서 기울기도 0 이라
// 구 가장자리의 정점이 갑자기 힘을 받지 않는다.
float makeGrabWeight(float distance, float radius)
{
	if (radius <= 0.0 || distance >= radius) {
		return 0.0;
	}
	float s = 1.0 - (distance * distance) / (radius * radius);
	return s * s;
}