#ifndef INCLUDE_UBO_H_
#define INCLUDE_UBO_H_

#include "common.h"

struct UniformBufferSkymapParams
{
    cVec4 lightPos[4] = { cVec4(0.0f) }; // 조명 위치
    cFloat exposure = 0.0f;
    cFloat gamma = 0.0f;
};

struct SceneDataUBO
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 cameraPos;
    cFloat padding1;
    glm::vec3 directionalLightDir{ -1.0f, -1.0f, -1.0f };
    cFloat padding2;
    glm::vec3 directionalLightColor{ 1.0f, 1.0f, 1.0f };
    cFloat padding3;
    glm::mat4 lightSpaceMatrix{ 1.0f };
};
// HDR skybox-specific control options
struct SkyOptionsUBO
{
    // HDR Environment mapping controls
    cFloat environmentIntensity = 1.0f; // Environment map intensity multiplier
    cFloat roughnessLevel = 0.5f;       // Mip level for prefiltered map (0.0 = sharpest)
    uint32_t useIrradianceMap = 0;     // 0 = use prefiltered, 1 = use irradiance

    // Skybox visualization and debug
    uint32_t showMipLevels = 0; // Visualize mip levels as colors
    uint32_t showCubeFaces = 0; // Visualize cube faces as colors
    cFloat padding1;
    cFloat padding2;
    cFloat padding3;
};

struct BoneDataUniform
{
    alignas(16) cMat4 boneMatrices[256]; // 16,384 bytes (already 16-byte aligned)
    alignas(16) cVec4 animationData;     // x = hasAnimation (0.0/1.0), y,z,w = future use
};

struct OptionsUniform
{
    alignas(4) cInt textureOn = 1;   // Use int instead of bool, 1 = true, 0 = false
    alignas(4) cInt shadowOn = 1;    // Use int instead of bool, 1 = true, 0 = false
    alignas(4) cInt discardOn = 1;   // Use int instead of bool, 1 = true, 0 = false
    alignas(4) cInt animationOn = 1; // Use int instead of bool, 1 = true, 0 = false
    alignas(4) cFloat ssaoRadius = 0.5f;
    alignas(4) cFloat ssaoBias = 0.025f;
    alignas(4) cInt ssaoSampleCount = 16;
    alignas(4) cFloat ssaoPower = 2.0f;
};


#endif // !INCLUDE_UBO_H_