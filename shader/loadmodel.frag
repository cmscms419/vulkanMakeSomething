#version 450

layout (binding = 1) uniform samplerCube samplerCubeMap;
layout (binding = 2) uniform sampler2D Sampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    //outColor = texture(texSampler, fragTexCoord * 5.0);
    outColor = vec4(fragColor * texture(Sampler, fragTexCoord).rgb, 1.0);
}
