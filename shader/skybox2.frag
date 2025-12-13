#version 450

layout(location = 0) in vec3 inLocalPos;

// Use std140 for predictable UBO layout when set from C++ side
layout(std140, set = 0, binding = 1) uniform SkyOptionsUBO {
    float environmentIntensity;
    float roughnessLevel;
    uint useIrradianceMap;
    uint showMipLevels;
    uint showCubeFaces;
    float padding1;
    float padding2;
    float padding3;
} skyOptions;

layout(set = 1, binding = 0) uniform samplerCube prefilteredMap;
layout(set = 1, binding = 1) uniform samplerCube irradianceMap;
layout(set = 1, binding = 2) uniform sampler2D brdfLUT;

layout(location = 0) out vec4 outColor;

// Debug visualization functions
vec3 getMipLevelColor(float mipLevel) {
    if (mipLevel < 1.0) return mix(vec3(1,0,0), vec3(1,1,0), mipLevel);
    else if (mipLevel < 2.0) return mix(vec3(1,1,0), vec3(0,1,0), mipLevel - 1.0);
    else if (mipLevel < 3.0) return mix(vec3(0,1,0), vec3(0,1,1), mipLevel - 2.0);
    else if (mipLevel < 4.0) return mix(vec3(0,1,1), vec3(0,0,1), mipLevel - 3.0);
    else return mix(vec3(0,0,1), vec3(1,0,1), clamp(mipLevel - 4.0, 0.0, 1.0));
}

vec3 getCubeFaceColor(vec3 dir) {
    vec3 absDir = abs(dir);
    if (absDir.x >= absDir.y && absDir.x >= absDir.z) {
        return dir.x > 0.0 ? vec3(1,0,0) : vec3(0,1,1); // +X: red, -X: cyan
    } else if (absDir.y >= absDir.x && absDir.y >= absDir.z) {
        return dir.y > 0.0 ? vec3(0,1,0) : vec3(1,0,1); // +Y: green, -Y: magenta
    } else {
        return dir.z > 0.0 ? vec3(0,0,1) : vec3(1,1,0); // +Z: blue, -Z: yellow
    }
}

void main() {
    // Normalize direction for cube map sampling
    vec3 dir = normalize(inLocalPos);
    vec3 envColor;

    // Sample environment map based on mode
    if (skyOptions.useIrradianceMap != 0u) {
        envColor = texture(irradianceMap, dir).rgb;
    } else {
        // textureLod uses mip level; roughnessLevel is used directly here
        envColor = textureLod(prefilteredMap, dir, skyOptions.roughnessLevel).rgb;
    }

    // Apply environment intensity
    envColor *= skyOptions.environmentIntensity;

    // Debug visualizations
    if (skyOptions.showMipLevels != 0u) {
        envColor = mix(envColor, getMipLevelColor(skyOptions.roughnessLevel), 0.5);
    }

    if (skyOptions.showCubeFaces != 0u) {
        envColor = mix(envColor, getCubeFaceColor(dir), 0.3);
    }

    // Output raw HDR values (exposure handled in post)
    outColor = vec4(envColor, 1.0);
}