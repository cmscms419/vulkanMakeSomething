#version 450

layout(location = 0) in vec3 normalWorld;
layout(location = 1) in vec3 posWorld;
layout(location = 2) in vec3 texcoord;
layout(location = 3) in vec4 color;
layout(location = 4) in vec4 posProj;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = color;
}