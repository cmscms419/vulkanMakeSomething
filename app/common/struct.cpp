#include "struct.h"
#include <glm/gtx/hash.hpp>

namespace std {
template<> struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const {
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

    template<> struct hash<Vertex2> {
        size_t operator()(Vertex2 const& vertex) const {
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

const std::vector<Vertex> DepthTestVertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}
};

const std::vector<Vertex> SquareTestVertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
};

const std::vector<Vertex> cube{
    // right face 
    { {0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 0.0f , 0.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f , 0.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},

    // left face
    { {-0.5f, 0.5f, 0.5f},   {-1.0f, 0.0f, 0.0f}, { 1.0f,  0.0f, 1.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {-0.5f, 0.5f, -0.5f},  {-1.0f, 0.0f, 0.0f}, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {-0.5f, -0.5f, 0.5f},  {-1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},

    // top face
    { {0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, { 1.0f, 0.0f , 2.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, { 1.0f, 1.0f, 2.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { { -0.5f, 0.5f, -0.5f }, {0.0f, 1.0f, 0.0f}, { 0.0f, 1.0f, 2.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, { 0.0f, 0.0f , 2.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},

    // bottom face
    { {-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, { 1.0f, 0.0f , 3.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { { -0.5f, -0.5f, -0.5f }, {0.0f, -1.0f, 0.0f}, { 1.0f, 1.0f, 3.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, { 0.0f, 1.0f, 3.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, { 0.0f, 0.0f , 3.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},

    // front face
    { {0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f, 4.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, { 1.0f, 1.0f, 4.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f , 4.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { { -0.5f, -0.5f, 0.5f }, {0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 4.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},

    // back face
    { {0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, { 0.0f, 0.0f, 5.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { { -0.5f, -0.5f, -0.5f }, {0.0f, 0.0f, -1.0f}, { 1.0f, 0.0f, 5.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, { 1.0f, 1.0f, 5.0f }, { 0.0f, 0.0f , 0.0f, 0.0f}},
    { {0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, { 0.0f, 1.0f, 5.0f}, { 0.0f, 0.0f , 0.0f, 0.0f}}
};

// https://github.com/SaschaWillems/Vulkan 에서 가져온 스카이박스 버텍스 데이터
// position , normal, texture coordinate
const std::vector<Vertex> skyboxVertices = {
    // back face
    {{-5.00f,  5.00f, -5.00f}, {0.00f, -1.19209e-07f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{-5.00f, -5.00f, -5.00f}, {0.00f, -1.19209e-07f, -1.00f}, {1.00f, 0.00f, 0.0f}},
    {{ 5.00f, -5.00f, -5.00f}, {0.00f, -1.19209e-07f, -1.00f}, {0.00f, 0.00f, 0.0f}},
    {{ 5.00f,  5.00f, -5.00f}, {0.00f, -1.19209e-07f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    // front face
    {{-5.00f,  5.00f,  5.00f}, {0.00f, 1.19209e-07f, 1.00f}, {0.00f, 1.00f, 0.0f}},
    {{ 5.00f,  5.00f,  5.00f}, {0.00f, 1.19209e-07f, 1.00f}, {1.00f, 1.00f, 0.0f}},
    {{ 5.00f, -5.00f,  5.00f}, {0.00f, 1.19209e-07f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{-5.00f, -5.00f,  5.00f}, {0.00f, 1.19209e-07f, 1.00f}, {0.00f, 0.00f, 0.0f}},
    // top face
    {{-5.00f,  5.00f, -5.00f}, {0.00005f, -1.62921e-07f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    {{ 5.00f,  5.00f, -5.00f}, {-0.00005f, -1.62921e-07f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{ 5.00f,  5.00f,  5.00f}, {-0.00005f, 7.54979e-08f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{-5.00f,  5.00f,  5.00f}, {0.00005f, 7.54979e-08f, 1.00f}, {0.00f, 0.00f, 0.0f}},
    // right face
    {{ 5.00f,  5.00f, -5.00f}, {-4.37114e-08f, -0.00005f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    {{ 5.00f, -5.00f, -5.00f}, {-4.37114e-08f, 0.00005f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{ 5.00f, -5.00f,  5.00f}, {-4.37114e-08f, 0.00005f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{ 5.00f,  5.00f,  5.00f}, {-4.37114e-08f, -0.00005f, 1.00f}, {0.00f, 0.00f, 0.0f}},
    // bottom face
    {{ 5.00f, -5.00f, -5.00f}, {-0.00005f, -7.54979e-08f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    {{-5.00f, -5.00f, -5.00f}, {0.00005f, -7.54979e-08f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{-5.00f, -5.00f,  5.00f}, {0.00005f, 1.62921e-07f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{ 5.00f, -5.00f,  5.00f}, {-0.00005f, 1.62921e-07f, 1.00f}, {0.00f, 0.00f, 0.0f}},
    // left face
    {{-5.00f, -5.00f, -5.00f}, {4.37114e-08f, 0.00005f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    {{-5.00f,  5.00f, -5.00f}, {4.37114e-08f, -0.00005f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{-5.00f,  5.00f,  5.00f}, {4.37114e-08f, -0.00005f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{-5.00f, -5.00f,  5.00f}, {4.37114e-08f, 0.00005f, 1.00f}, {0.00f, 0.00f, 0.0f}},
};

const std::vector<cUint32_t> DepthTestIndices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
};

const std::vector<cUint32_t> SquareTestIndices_ = {
        0, 1, 2, 2, 3, 0
};

const std::vector<cUint32_t> cubeindices_ = {
       0, 1, 2, 2, 3, 0,
       4, 5, 6, 6, 7, 4,
       8, 9, 10, 10, 11, 8,
       12, 13, 14, 14, 15, 12,
       16, 17, 18, 18, 19, 16,
       20, 21, 22, 22, 23, 20
};

const std::vector<cUint32_t> skyboxIndices = {
    0, 1, 2, 2, 3, 0,   // left face
    4, 5, 6, 6, 7, 4,   // right face
    8, 9, 10, 10, 11, 8,// top face
    12, 13, 14, 14, 15, 12, // bottom face
    16, 17, 18, 18, 19, 16, // back face
    20, 21, 22, 22, 23, 20 // front face
};

std::array<VkVertexInputAttributeDescription, 4> Vertex2::getAttributeDescriptionsBasic()
{
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, inTangent);

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

void Vertex2::addBoneData(cUint32_t boneIndex, cFloat weight)
{
    // Find an empty slot (boneIndex == -1) to add the bone data
    for (int i = 0; i < 4; ++i) {
        if (BoneIndices[i] == -1) {
            BoneIndices[i] = static_cast<int>(boneIndex);
            BoneWeights[i] = weight;
            return;
        }
    }

    // If no empty slot found, replace the bone with the smallest weight
    // if the new weight is larger
    int minIndex = 0;
    for (int i = 1; i < 4; ++i) {
        if (BoneWeights[i] < BoneWeights[minIndex]) {
            minIndex = i;
        }
    }

    if (weight > BoneWeights[minIndex]) {
        BoneIndices[minIndex] = static_cast<int>(boneIndex);
        BoneWeights[minIndex] = weight;
    }
}

void Vertex2::normalizeBoneWeights()
{
    cFloat totalWeight = BoneWeights.x + BoneWeights.y + BoneWeights.z + BoneWeights.w;

    if (totalWeight > 0.0f) {
        BoneWeights /= totalWeight;
    }
    else {
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

Plane::Plane(const cVec3& p1, const cVec3& p2, const cVec3& p3)
{
    cVec3 v1 = p2 - p1;
    cVec3 v2 = p3 - p1;
    normal = glm::normalize(glm::cross(v1, v2));
    distance = -glm::dot(normal, p1);
}

Plane::Plane(const cVec3& normal, const cVec3& point) 
    : normal(glm::normalize(normal))
{
    distance = -glm::dot(this->normal, point);
}

cFloat Plane::distanceToPoint(const cVec3& point) const
{
    return glm::dot(normal, point) + distance;
}

AABB AABB::transform(const cMat4& matrix) const
{
    // Transform all 8 corners and find new min/max
    cVec3 corners[8] = { {min.x, min.y, min.z}, {max.x, min.y, min.z}, {min.x, max.y, min.z},
                            {max.x, max.y, min.z}, {min.x, min.y, max.z}, {max.x, min.y, max.z},
                            {min.x, max.y, max.z}, {max.x, max.y, max.z} };

    cVec3 newMin(FLT_MAX);
    cVec3 newMax(-FLT_MAX);

    for (int i = 0; i < 8; ++i) {
        cVec4 transformed = matrix * cVec4(corners[i], 1.0f);
        cVec3 point = cVec3(transformed) / transformed.w;

        newMin = (glm::min)(newMin, point);
        newMax = (glm::max)(newMax, point);
    }

    return AABB(newMin, newMax);
}
