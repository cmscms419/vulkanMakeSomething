#ifndef INCLUDE_MATERIAL_H_
#define INCLUDE_MATERIAL_H_

#include "common.h"

struct cMaterial
{
    cFloat metallic = 0.0f; // Metallic factor
    cFloat roughness = 0.0f; // Roughness factor
    cFloat r = 0.0f; // Red color component
    cFloat g = 0.0f; // Green color component
    cFloat b = 0.0f; // Blue color component
    cFloat a = 1.0f; // Alpha component (opacity)
    VkBool32 isAlbedoMap = VK_FALSE; // Albedo map flag
    VkBool32 isNormalMap = VK_FALSE; // Normal map flag
    VkBool32 isMetallic = VK_FALSE; // Metallic flag
    VkBool32 isRoughness = VK_FALSE; // Roughness flag

    cUint32_t baseColorTextureIndex = 0;

#if 0
    cFloat emissive = 0.0f; // Emissive factor
    cFloat alphaCutoff = 0.5f; // Alpha cutoff value for transparency
    cBool isTransparent = false; // Transparency flag
    cBool isEmissive = false; // Emissive flag
    cBool isOcclusionMap = false; // Occlusion map flag
    cBool isSpecularMap = false; // Specular map flag
    cBool isEmissiveMap = false; // Emissive map flag
    cBool isAlphaCutoff = false; // Alpha cutoff flag
    cBool isAlphaBlend = false; // Alpha blend flag
    cBool isAlphaTest = false; // Alpha test flag
    cBool isDoubleSided = false; // Double-sided rendering flag
    cBool isUseTexture = false; // Texture usage flag
    cBool isUseNormalMap = false; // Normal map usage flag
    cBool isUseOcclusionMap = false; // Occlusion map usage flag
    cBool isUseAlbedoMap = false; // Albedo map usage flag
    cBool isUseSpecularMap = false; // Specular map usage flag
    cBool isUseEmissiveMap = false; // Emissive map usage flag
    cBool isUseAlphaCutoff = false; // Alpha cutoff usage flag
    cBool isUseAlphaBlend = false; // Alpha blend usage flag
    cBool isUseAlphaTest = false; // Alpha test usage flag
    cBool isUseDoubleSided = false; // Double-sided rendering usage flag
    cBool isUseMetallic = false; // Metallic usage flag
    cBool isUseRoughness = false; // Roughness usage flag
    cBool isUseEmissive = false; // Emissive usage flag
    cBool isUseColor = false; // Color usage flag
    cBool isUseSpecular = false; // Specular usage flag
    cBool isUseTextureResource = false; // Texture resourcePNG usage flag
#endif

    cMaterial(cFloat metallic = 0.0f, cFloat roughness = 0.0f, cVec4 color = cVec4(0.0f, 0.0f, 0.0f, 1.0f))
        : metallic(metallic), roughness(roughness), r(color.r), g(color.g), b(color.b), a(color.a) {

    }
};

struct cMaterial2
{
    alignas(16) cVec4 emissiveFactor = cVec4(0.0f);
    alignas(16) cVec4 baseColorFactor = cVec4(1.0f);
    alignas(4) cFloat roughness = 1.0f;
    alignas(4) cFloat transparencyFactor = 1.0f;
    alignas(4) cFloat discardAlpha = 0.0f;
    alignas(4) cFloat metallicFactor = 0.0f;
    alignas(4) cInt baseColorTextureIndex = -1;
    alignas(4) cInt emissiveTextureIndex = -1;
    alignas(4) cInt normalTextureIndex = -1;
    alignas(4) cInt opacityTextureIndex = -1;
    alignas(4) cInt metallicRoughnessTextureIndex = -1;
    alignas(4) cInt occlusionTextureIndex = -1;
};

#endif // !INCLUDE_MATERIAL_H_