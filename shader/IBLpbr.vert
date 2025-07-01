#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 inverseTranspose;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConsts {
	vec3 objectPos;
} pushConsts;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragTexCoord;
layout(location = 2) out vec3 WorldPos;

void main() {
    vec3 localPos = vec3(ubo.model * vec4(inPosition, 1.0));
    WorldPos = localPos + pushConsts.objectPos;
    
    fragNormal = normalize(mat3(ubo.inverseTranspose) * inNormal);
    fragTexCoord = vec3(inTexCoord.x,1.0 - inTexCoord.y, 0.0);

    gl_Position = ubo.proj * ubo.view * vec4(localPos, 1.0);
}