#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "../../common/common.h"
#include "../../common/struct.h"
#include "../../common/macros.h"


struct VkBufferObject {
    
    VkDevice device = VK_NULL_HANDLE; ///< Vulkan ��ġ �ڵ�
    VkBuffer buffer = VK_NULL_HANDLE; ///< Vulkan ���� �ڵ�
    VkDeviceMemory memory = VK_NULL_HANDLE; ///< Vulkan ��ġ �޸� �ڵ�
    VkDescriptorBufferInfo descriptor{}; ///< Vulkan ��ũ���� ���� ����
    VkDeviceSize size = 0; ///< ���� ũ��
    VkDeviceSize alignment = 0; ///< ���� ����

    VkBufferUsageFlags usageFlags; ///< ���� ��� �÷���
    VkMemoryPropertyFlags memoryPropertyFlags; ///< �޸� �Ӽ� �÷���
    void* mapped = nullptr; ///< ���ε� �޸� ������

    void cleanup(); ///< ���� ���� �Լ�
    VkResult mapToMeBuffer(VkDeviceSize size, VkDeviceSize offset); ///< ���� ���� �Լ�
    void copyToMeBuffer(void* data, VkDeviceSize size); ///< ���۷� ������ ���� �Լ�
    void createBuffer(VkPhysicalDevice physicalDevice); ///< ���� ���� �Լ�
};

struct VertexBuffer : public VkBufferObject
{
    Vertex vertexBuffer{}; ///< ���ؽ� ����
};

struct IndexBuffer : public VkBufferObject 
{

};


#endif // !INCLUDE_VKBUFFER_H_
