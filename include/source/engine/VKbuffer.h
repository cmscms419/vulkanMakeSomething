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
}
#endif // !INCLUDE_VKBUFFER_H_
