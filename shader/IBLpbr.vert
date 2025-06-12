#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 3) uniform subUinform {
    vec4 camPos;
    vec4 lightPos;
    vec4 objectPos;
} another;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragTexCoord;
layout(location = 2) out vec3 WorldPos;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    WorldPos = another.objectPos.xyz;
    fragNormal = inNormal;
    fragTexCoord = inTexCoord;
}