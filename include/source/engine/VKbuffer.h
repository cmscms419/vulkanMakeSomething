#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

namespace vkengine {

    struct VkBufferObject {

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
