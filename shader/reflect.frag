#version 450

layout (binding = 2) uniform sampler2DArray samplerArray;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;


void main() {

    outColor = texture(samplerArray, fragTexCoord);
    //outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);
    //outColor = vec4(fragTexCoord, 0.0, 1.0);
    //outColor = vec4(fragColor, 1.0);
}
