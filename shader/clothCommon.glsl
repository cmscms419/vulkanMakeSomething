struct clothVertex {
	vec3 position; 		float mass;
	vec3 prevPosition;	float pinned;
	vec3 nextPosition;	float padding1;
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