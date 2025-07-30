#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTexCoord;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec3 fragColor;

void main() {

    gl_PointSize = 14.0;
    gl_Position = vec4(inPosition.xy, 1.0, 1.0);
    fragColor = inTexCoord.rgb;
}