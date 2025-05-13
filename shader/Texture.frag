#version 450

layout(binding = 1) uniform sampler2D Sampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(Sampler, fragTexCoord.xy).rgb, 1.0);
}