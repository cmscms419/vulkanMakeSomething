#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

namespace vkengine {

    struct VkBufferObject {

        VkDevice device = VK_NULL_HANDLE; ///< Vulkan ��ġ �ڵ�
        VkBuffer buffer = VK_NULL_HANDLE; ///< Vulkan ���� �ڵ�
        VkDeviceMemory memory = VK_NULL_HANDLE; ///< Vulkan ��ġ �޸� �ڵ�
        VkDescriptorBufferInfo descriptor{}; ///< Vulkan ��ũ���� ���� ����
        VkDeviceSize size = 0; ///< ���� ũ��
        VkDeviceSize alignment = 0; ///< ���� ����

        VkBufferUsageFlags usageFlags = 0; ///< ���� ��� �÷���
        VkMemoryPropertyFlags memoryPropertyFlags = 0; ///< �޸� �Ӽ� �÷���
        void* mapped = nullptr; ///< ���ε� �޸� ������

        void cleanup(); ///< ���� ���� �Լ�
        VkResult mapToMeBuffer(VkDeviceSize size, VkDeviceSize offset); ///< ���� ���� �Լ�
        void copyToMeBuffer(void* data, VkDeviceSize size); ///< ���۷� ������ ���� �Լ�
        void createBuffer(VkPhysicalDevice physicalDevice); ///< ���� ���� �Լ�
        
        virtual void createDescriptorBufferInfo() ///< ��ũ���� ���� ���� ���� �Լ�
        {
            this->descriptor.buffer = this->buffer;
            this->descriptor.offset = 0;
            this->descriptor.range = this->size;
        };
    };

    struct VertexBuffer : public VkBufferObject
    {
        std::vector<Vertex> vertices;
    };

    struct IndexBuffer : public VkBufferObject
    {
        std::vector<uint16_t> indices;
        void setIndices(std::vector<uint16_t>& indices) { this->indices = indices; }
    };

    struct UniformBuffer : public VkBufferObject
    {
        UniformBufferObject trs = {};

        UniformBuffer()
        {
            this->size = sizeof(UniformBufferObject);
        };

    };

    struct UniformBufferSkymap : public VkBufferObject
    {
        UniformBufferSkymapParams uboParams = {};

        UniformBufferSkymap()
        {
            this->size = sizeof(UniformBufferSkymapParams);
        };
    };

    struct MaterialBuffer : public VkBufferObject
    {
        Material material = {};
        cString name;
        MaterialBuffer(
            cString name = "DefaultMaterial",
            cFloat metallic = 0.0f,
            cFloat roughness = 0.0f,
            cVec4 color = cVec4(0.0f, 0.0f, 0.0f, 1.0f)
        )
        {
            this->size = sizeof(Material);
            this->name = name;
            this->material = Material(metallic, roughness, color);
        };
    };
}

#endif // !INCLUDE_VKBUFFER_H_
