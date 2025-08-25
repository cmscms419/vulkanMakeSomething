#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

namespace vkengine {

    struct VkBaseBuffer {
        cString name = "Default";
        VkDevice device = VK_NULL_HANDLE; ///< Vulkan 장치 핸들
        VkBuffer buffer = VK_NULL_HANDLE; ///< Vulkan 버퍼 핸들
        VkDeviceMemory memory = VK_NULL_HANDLE; ///< Vulkan 장치 메모리 핸들
        VkDescriptorBufferInfo descriptor{}; ///< Vulkan 디스크립터 버퍼 정보
        VkDeviceSize size = 0; ///< 버퍼 크기
        VkDeviceSize alignment = 0; ///< 버퍼 정렬

        VkBufferUsageFlags usageFlags = 0; ///< 버퍼 사용 플래그
        VkMemoryPropertyFlags memoryPropertyFlags = 0; ///< 메모리 속성 플래그
        void* mapped = nullptr; ///< 매핑된 메모리 포인터

        void cleanup(); ///< 버퍼 정리 함수
        VkResult mapToMeBuffer(VkDeviceSize size, VkDeviceSize offset); ///< 버퍼 매핑 함수
        void copyToMeBuffer(void* data, VkDeviceSize size); ///< 버퍼로 데이터 복사 함수
        void createBuffer(VkPhysicalDevice physicalDevice); ///< 버퍼 생성 함수
        
        virtual void createDescriptorBufferInfo() ///< 디스크립터 버퍼 정보 생성 함수
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
