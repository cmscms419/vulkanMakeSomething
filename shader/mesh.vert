#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inUV;
layout(location = 3) in vec4 inColor;

layout (set = 0, binding = 0) uniform UBOScene
{
    mat4 projection;
    mat4 view;
    vec4 lightPos;
    vec4 viewPos;
} uboScene;

layout(std140, push_constant) uniform PushConsts {
    layout (offset = 0) mat4 model;
} primitive;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;

void main() 
{
    outNormal = inNormal;
    outColor = inColor.xyz;
    outUV = inUV.xy;
    // outUV.y = 1.0 - outUV.y; // Flip Y coordinate for correct texture mapping

    vec3 localPos = vec3(primitive.model * vec4(inPos.xyz, 1.0));

    gl_Position = uboScene.projection * uboScene.view * vec4(localPos, 1.0);
    //gl_Position.y = -gl_Position.y; // Flip Y coordinate for correct rendering

    vec4 pos = uboScene.view * vec4(inPos, 1.0);
    outNormal = mat3(uboScene.view) * inNormal;
    vec3 lPos = mat3(uboScene.view) * uboScene.lightPos.xyz;
    outLightVec = uboScene.lightPos.xyz - pos.xyz;
    outViewVec = uboScene.viewPos.xyz - pos.xyz;	
}