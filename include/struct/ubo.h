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
};

struct SSAOParamsUBO
{
    alignas(4) cFloat ssaoRadius = 0.5f;
    alignas(4) cFloat ssaoBias = 0.025f;
    alignas(4) cInt ssaoSampleCount = 16;
    alignas(4) cFloat ssaoPower = 2.0f;
};

// Post-processing options uniform buffer structure
struct PostProcessingOptionsUBO
{
    // Tone mapping options
    cInt toneMappingType = 2; // 0=None, 1=Reinhard, 2=ACES, 3=Uncharted2, 4=GT, 5=Lottes, 6=Exponential, 7=ReinhardExtended, 8=Luminance, 9=Hable
    cFloat exposure = 1.0f;       // HDR exposure adjustment
    cFloat gamma = 2.2f;          // Gamma correction value
    cFloat maxWhite = 11.2f;      // For extended Reinhard tone mapping

    // Color grading
    cFloat contrast = 1.0f;   // Contrast adjustment
    cFloat brightness = 0.0f; // Brightness adjustment
    cFloat saturation = 1.0f; // Color saturation
    cFloat vibrance = 0.0f;   // Vibrance (smart saturation)

    // Effects
    cFloat vignetteStrength = 0.0f;    // Vignette effect strength
    cFloat vignetteRadius = 0.8f;      // Vignette radius
    cFloat filmGrainStrength = 0.0f;   // Film grain noise strength
    cFloat chromaticAberration = 0.0f; // Chromatic aberration strength

    // Debug and visualization
    cInt debugMode =
        0; // 0=Off, 1=Show tone mapping comparison, 2=Show color channels, 3=Split comparison
    cFloat debugSplit = 0.5f;     // Split position for comparison (0.0-1.0)
    cInt showOnlyChannel = 0; // 0=All, 1=Red, 2=Green, 3=Blue, 4=Alpha, 5=Luminance
    cFloat padding1 = 0.0f;
};

// Push constant structure for per-draw data
// Matches shader layout: mat4 model(64) + float coeffs[15](60) + uint materialIndex(4) = 128 bytes
struct DrawModelResource {
    cMat4    modelMatrix;       // model transform matrix
    cFloat    coeffs[15];       // [0]=specular, [1]=diffuse, [2]=emissive,
                                // [3]=shadowOffset, [4]=metallic, [5]=roughness,
                                // [6-14]= 아직 정해지 않음
    uint32_t materialIndex;     // index into material array (was coeffs[15])
};
static_assert(sizeof(DrawModelResource) == 128, "DrawModelResource must be 128 bytes");

#endif // !INCLUDE_UBO_H_