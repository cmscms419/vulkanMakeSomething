#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

namespace vkengine {

    struct VkBaseBuffer {
        cString name = "Default";
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

        VkBaseBuffer(VkDeviceSize size = 0, cString name = "Default")
            : name(name), size(size) {}

    };

    struct VertexBuffer : public VkBaseBuffer
    {
        VertexBuffer(VkDeviceSize size = sizeof(Vertex), cString name = "DefaultVertext")
        {
            VkBaseBuffer buffer(size, name);
        };
    };

    struct IndexBuffer : public VkBaseBuffer
    {
        IndexBuffer(VkDeviceSize size = sizeof(cUint32_t), cString name = "DefaultIndexBuffer")
        {
            VkBaseBuffer buffer(size, name);
        };
    };

    struct UniformBuffer : public VkBaseBuffer
    {
        UniformBuffer(VkDeviceSize size = sizeof(UniformBufferObject), cString name = "DefaultUniformBuffer")
        {
            VkBaseBuffer buffer(size, name);
        };
    };

    struct UniformBufferSkymap : public VkBaseBuffer
    {
        UniformBufferSkymap(VkDeviceSize size = sizeof(UniformBufferObject), cString name = "DefaultUniformBufferSkymap")
        {
            VkBaseBuffer buffer(size, name);
        };
    };

    struct MaterialBuffer : public VkBaseBuffer
    {
        MaterialBuffer(VkDeviceSize size = sizeof(cMaterial), cString name = "DefaultMaterial")
        {
            VkBaseBuffer buffer(size, name);
        };
    };

    struct PaterialBuffer : public VkBaseBuffer
    {
        PaterialBuffer(VkDeviceSize size = sizeof(Particle), cString name = "DefaultPaterial")
        {
            VkBaseBuffer buffer(size, name);
        };
    };
}

#endif // !INCLUDE_VKBUFFER_H_
