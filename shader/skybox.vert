#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 Color;
layout (location = 2) in vec3 inTexCoord;
layout(location = 3) in vec4 inTangent;

layout (binding = 0) uniform UBO 
{
    mat4 model;
    mat4 inverseTranspose;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) out vec3 outUVW;

void main() 
{
    outUVW = inPos;
    
    mat4 viewWithoutTranslation = mat4(mat3(ubo.view));
    gl_Position = ubo.proj * viewWithoutTranslation * vec4(inPos.xyz, 1.0);
}
