#include "type.h"
#include <glm/gtx/hash.hpp>

namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(Vertex const &vertex) const
        {
            size_t h1 = hash<cVec3>()(vertex.pos);
            size_t h2 = hash<cVec3>()(vertex.normal);
            size_t h3 = hash<cVec3>()(vertex.texCoord);
            size_t h4 = hash<cVec4>()(vertex.inTangent);

            size_t seed = 0;
            seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    template <>
    struct hash<Vertex2>
    {
        size_t operator()(Vertex2 const &vertex) const
        {
            size_t h1 = hash<cVec3>()(vertex.pos);
            size_t h2 = hash<cVec3>()(vertex.normal);
            size_t h3 = hash<cVec2>()(vertex.texCoord);
            size_t h4 = hash<cVec3>()(vertex.inTangent);
            size_t h5 = hash<cVec3>()(vertex.Bitangent);
            size_t h6 = hash<cVec4>()(vertex.BoneWeights);
            size_t h7 = hash<cIvec4>()(vertex.BoneIndices);

            size_t seed = 0;
            seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h5 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h6 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h7 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}

std::array<VkVertexInputAttributeDescription, 4> Vertex2::getAttributeDescriptionsBasic()
{
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex2, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex2, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex2, texCoord);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex2, inTangent);

    return attributeDescriptions;
}

std::array<VkVertexInputAttributeDescription, 7> Vertex2::getAttributeDescriptionsAnimated()
{
    std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};

    // Position attribute (location = 0)
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex2, pos);

    // Normal attribute (location = 1)
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex2, normal);

    // Texture coordinate attribute (location = 2)
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex2, texCoord);

    // Tangent attribute (location = 3)
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex2, inTangent);

    // Bitangent attribute (location = 4)
    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex2, Bitangent);

    // Bone weights attribute (location = 5)
    attributeDescriptions[5].binding = 0;
    attributeDescriptions[5].location = 5;
    attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[5].offset = offsetof(Vertex2, BoneWeights);

    // Bone indices attribute (location = 6)
    attributeDescriptions[6].binding = 0;
    attributeDescriptions[6].location = 6;
    attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SINT;
    attributeDescriptions[6].offset = offsetof(Vertex2, BoneIndices);

    return attributeDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex2::getAttributeDescriptionsAnimatedVec()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    attributeDescriptions.resize(7);

    // Position attribute(location = 0)
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex2, pos);

    // Normal attribute (location = 1)
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex2, normal);

    // Texture coordinate attribute (location = 2)
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex2, texCoord);

    // Tangent attribute (location = 3)
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex2, inTangent);

    // Bitangent attribute (location = 4)
    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex2, Bitangent);

    // Bone weights attribute (location = 5)
    attributeDescriptions[5].binding = 0;
    attributeDescriptions[5].location = 5;
    attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[5].offset = offsetof(Vertex2, BoneWeights);

    // Bone indices attribute (location = 6)
    attributeDescriptions[6].binding = 0;
    attributeDescriptions[6].location = 6;
    attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SINT;
    attributeDescriptions[6].offset = offsetof(Vertex2, BoneIndices);

    return attributeDescriptions;
}

void Vertex2::addBoneData(cUint32_t boneIndex, cFloat weight)
{
    // Find an empty slot (boneIndex == -1) to add the bone data
    for (int i = 0; i < 4; ++i)
    {
        if (BoneIndices[i] == -1)
        {
            BoneIndices[i] = static_cast<int>(boneIndex);
            BoneWeights[i] = weight;
            return;
        }
    }

    // If no empty slot found, replace the bone with the smallest weight
    // if the new weight is larger
    int minIndex = 0;
    for (int i = 1; i < 4; ++i)
    {
        if (BoneWeights[i] < BoneWeights[minIndex])
        {
            minIndex = i;
        }
    }

    if (weight > BoneWeights[minIndex])
    {
        BoneIndices[minIndex] = static_cast<int>(boneIndex);
        BoneWeights[minIndex] = weight;
    }
}

void Vertex2::normalizeBoneWeights()
{
    cFloat totalWeight = BoneWeights.x + BoneWeights.y + BoneWeights.z + BoneWeights.w;

    if (totalWeight > 0.0f)
    {
        BoneWeights /= totalWeight;
    }
    else
    {
        // If no bone weights, set default (no animation)
        BoneWeights = cVec4(0.0f);
        BoneIndices = cIvec4(-1);
    }
}

bool Vertex2::hasValidBoneData() const
{
    return (
        this->BoneIndices.x >= 0 ||
        this->BoneIndices.y >= 0 ||
        this->BoneIndices.z >= 0 ||
        this->BoneIndices.w >= 0);
}