#ifndef INCLUDE_TYPE_H_
#define INCLUDE_TYPE_H_

#include <array>
#include <vector>

#include "common.h"
#include "macros.h"

struct Vertex {
    cVec3 pos;
    cVec3 normal;
    cVec3 texCoord;
    cVec4 inTangent;

    // 바인딩 설명을 반환하는 함수
    // 이 구조체의 멤버 변수가 어떻게 바인딩되는지 설명합니다.
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    // 어트리뷰트 설명을 반환하는 함수
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
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

    cBool operator==(const Vertex& other) const {
        return
            pos == other.pos &&
            normal == other.normal &&
            texCoord == other.texCoord &&
            inTangent == other.inTangent;
    }

};

struct Vertex2 {
    alignas(4) cVec3 pos;
    alignas(4) cVec3 normal;
    alignas(4) cVec2 texCoord;
    alignas(4) cVec3 inTangent;
    alignas(4) cVec3 Bitangent;

    // Skeletal animation support
    alignas(4) cVec4 BoneWeights;  // 16 bytes - bone influence weights (up to 4 bones per vertex)
    alignas(4) cIvec4 BoneIndices; // 16 bytes - bone indices (up to 4 bones per vertex)

    // Default constructor
    Vertex2()
        : pos(0.0f), normal(0.0f, 1.0f, 0.0f), texCoord(0.0f), inTangent(1.0f, 0.0f, 0.0f),
        Bitangent(0.0f, 0.0f, 1.0f), BoneWeights(0.0f), BoneIndices(-1)
    {
    }

    // Constructor for regular vertices (without animation)
    Vertex2(const cVec3& pos, const cVec3& norm, const cVec2& tex)
        : pos(pos), normal(norm), texCoord(tex), inTangent(1.0f, 0.0f, 0.0f),
        Bitangent(0.0f, 0.0f, 1.0f), BoneWeights(0.0f), BoneIndices(-1)
    {
    }

    // Constructor for animated vertices
    Vertex2(const cVec3& pos, const cVec3& norm, const cVec2& tex, const cVec4& weights,
        const cIvec4& indices)
        : pos(pos), normal(norm), texCoord(tex), inTangent(1.0f, 0.0f, 0.0f),
        Bitangent(0.0f, 0.0f, 1.0f), BoneWeights(weights), BoneIndices(indices)
    {
    }

    // 바인딩 설명을 반환하는 함수
    // 이 구조체의 멤버 변수가 어떻게 바인딩되는지 설명합니다.
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex2);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    // 어트리뷰트 설명을 반환하는 함수
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptionsBasic();
    static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptionsAnimated();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptionsAnimatedVec();

    void addBoneData(cUint32_t boneIndex, cFloat weight);
    void normalizeBoneWeights();
    bool hasValidBoneData() const;

    cBool operator==(const Vertex2& other) const {
        return
            pos == other.pos &&
            normal == other.normal &&
            texCoord == other.texCoord &&
            inTangent == other.inTangent &&
            Bitangent == other.Bitangent &&
            BoneWeights == other.BoneWeights &&
            BoneIndices == other.BoneIndices;
    }

};

struct Particle {
    cVec3 position;
    cVec3 velocity;
    cVec3 color;
    cVec4 empty;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Particle);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Particle, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Particle, velocity);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Particle, color);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Particle, empty);

        return attributeDescriptions;
    }
};

struct InstanceData {
    alignas(4) cVec4 pos; // Transformation matrix (Translation, Rotation, Scale)

    InstanceData() : pos(cVec4(1.0f)) {} // Default constructor initializes to identity matrix
    InstanceData(const cVec4& pos) : pos(pos) {} // Constructor with transformation matrix
    InstanceData(const cVec3& pos) {
        cVec4 translation = cVec4(pos, 1.0f); // Convert cVec3 to cVec4 with w = 1.0
        this->pos = translation;
    }
};

struct LineVertex {
    cVec3 pos;
    cVec3 color;
};

// Hash function for vector<VkDescriptorSetLayoutBinding>
struct BindingHash
{
    size_t operator()(const std::vector<VkDescriptorSetLayoutBinding>& bindings) const;
};

// Equality function for vector<VkDescriptorSetLayoutBinding>
struct BindingEqual
{
    bool operator()(const std::vector<VkDescriptorSetLayoutBinding>& lhs,
        const std::vector<VkDescriptorSetLayoutBinding>& rhs) const;
};

#endif // !INCLUDE_TYPE_H_