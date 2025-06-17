#version 450

// Source code
// https://github.com/SaschaWillems/Vulkan/blob/master/shaders/glsl/texturecubemap/reflect.vert

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 InNormal;
layout(location = 2) in vec3 inTexCoord;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec3 outLightVec;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    outPos = vec3(ubo.view * ubo.model * vec4(inPosition, 1.0));
    outNormal = mat3(ubo.view * ubo.model) * InNormal;

    vec3 lightPos = vec3(0.0f, -5.0f, 5.0f);
    outLightVec = lightPos.xyz - outPos.xyz;
    outViewVec = -outPos.xyz;
}