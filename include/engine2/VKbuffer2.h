#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "common.h"

#include "VKContext.h"
#include "VKShaderResource.h"
#include "VKResourceBindingData.h"

namespace vkengine
{

    class VKBaseBuffer2 : public VKShaderResource
    {
    public:
        VKBaseBuffer2(VKcontext &ctx);

        // 이동 연산자 가능
        // VKBaseBuffer2 a = std::move(b) 가능
        VKBaseBuffer2(VKBaseBuffer2 &&) noexcept;

        // 복사 x
        // VKBaseBuffer2 a = b 불가능
        VKBaseBuffer2(const VKBaseBuffer2 &) = delete;

        // 복사 대입 연산자 x
        // a = b 불가능
        VKBaseBuffer2 &operator=(const VKBaseBuffer2 &) = delete;

        // 이동 대입 연산자 x
        // a = std::move(b) x
        VKBaseBuffer2 &operator=(VKBaseBuffer2 &&) = delete;

        ~VKBaseBuffer2()
        {
            cleanup();
        }

        void cleanup(); ///< 버퍼 정리 함수

        void createVertexBuffer(VkDeviceSize size, void *data);
        void createIndexBuffer(VkDeviceSize size, void *data);
        void createStagingBuffer(VkDeviceSize size, void *data);
        void createUniformBuffer(VkDeviceSize size, void *data);
        void createModelVertexBuffer(VkDeviceSize size, void *data);
        void createModeIndexBuffer(VkDeviceSize size, void *data);

        void updateData(const void *data, VkDeviceSize size, VkDeviceSize offset);
        void flush() const;

        virtual void createDescriptorBufferInfo() ///< 디스크립터 버퍼 정보 생성 함수
        {
            this->descriptor.buffer = this->buffer;
            this->descriptor.offset = 0;
            this->descriptor.range = this->size;
        };

        auto Buffer() -> VkBuffer &
        {
            return this->buffer;
        }

        auto Mapped() const -> void *
        {
            return this->mapped;
        }

        VKResourceBinding &getResourceBinding() override
        {
            return this->resourceBinding;
        }

        const VKResourceBinding &getResourceBinding() const override
        {
            return this->resourceBinding;
        }

        void updateBinding(VkDescriptorSetLayoutBinding &binding) override
        {
            binding.descriptorType = resourceBinding.descriptorType; // UBO / SSBO
            binding.descriptorCount = 1;
            binding.stageFlags = 0; // ShaderManager가 stageFlags 주입
        }

        void updateWrite(VkWriteDescriptorSet &write) override
        {
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = resourceBinding.descriptorType;
            write.descriptorCount = 1;
            write.pBufferInfo = &resourceBinding.bufferInfo;
        }

    private:
        void create(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
                    VkDeviceSize size, void *data);

        cString name;
        VKcontext &ctx;

        VkBuffer buffer;                     //< Vulkan 버퍼 핸들
        VkDeviceMemory memory;               ///< Vulkan 장치 메모리 핸들
        VkDescriptorBufferInfo descriptor{}; ///< Vulkan 디스크립터 버퍼 정보
        VkDeviceSize size;                   ///< 버퍼 크기
        VkDeviceSize offset;                 ///< 버퍼 간격
        VkDeviceSize allocatedSize;          ///< createBuffer 할 때, 만들어지는 버퍼의 크기
        VkDeviceSize alignment;              ///< 버퍼 정렬

        VkBufferUsageFlags usageFlags;             ///< 버퍼 사용 플래그
        VkMemoryPropertyFlags memoryPropertyFlags; ///< 메모리 속성 플래그
        void *mapped;                              ///< 매핑된 메모리 포인터

        VKResourceBinding resourceBinding;
    };
}

#endif // !INCLUDE_VKBUFFER_H_
