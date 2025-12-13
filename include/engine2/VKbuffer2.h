#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKContext.h"
#include "VKResourceBindingData.h"

namespace vkengine {

    class VKBaseBuffer2 {
    public:
        VKBaseBuffer2(VKcontext& ctx);

        // 이동 연산자 가능
        // VKBaseBuffer2 a = std::move(b) 가능
        VKBaseBuffer2(VKBaseBuffer2&&) noexcept;

        // 복사 x
        // VKBaseBuffer2 a = b 불가능
        VKBaseBuffer2(const VKBaseBuffer2&) = delete;

        // 복사 대입 연산자 x
        // a = b 불가능
        VKBaseBuffer2& operator=(const VKBaseBuffer2&) = delete;
        
        // 이동 대입 연산자 x
        // a = std::move(b) x
        VKBaseBuffer2& operator=(VKBaseBuffer2&&) = delete;

        ~VKBaseBuffer2() {
            cleanup();
        }

        void cleanup(); ///< 버퍼 정리 함수

        void createVertexBuffer(VkDeviceSize size, void* data);
        void createIndexBuffer(VkDeviceSize size, void* data);
        void createStagingBuffer(VkDeviceSize size, void* data);
        void createUniformBuffer(VkDeviceSize size, void* data);

        void updateData(const void* data, VkDeviceSize size, VkDeviceSize offset);
        void flush() const;

        virtual void createDescriptorBufferInfo() ///< 디스크립터 버퍼 정보 생성 함수
        {
            this->descriptor.buffer = this->buffer;
            this->descriptor.offset = 0;
            this->descriptor.range = this->size;
        };

        auto Buffer() -> VkBuffer& {
            return this->buffer;
        }

        auto Mapped() const -> void* {
            return this->mapped;
        }

        auto ResourceBinding() -> VKResourceBinding& {
            return this->resourceBinding;
        }

    private:
        void create(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize size, void* data);

        cString name = "Default";
        VKcontext& ctx;

        VkBuffer buffer = VK_NULL_HANDLE; //< Vulkan 버퍼 핸들
        VkDeviceMemory memory = VK_NULL_HANDLE; ///< Vulkan 장치 메모리 핸들
        VkDescriptorBufferInfo descriptor{}; ///< Vulkan 디스크립터 버퍼 정보
        VkDeviceSize size = 0; ///< 버퍼 크기
        VkDeviceSize offset = 0; ///< 버퍼 간격
        VkDeviceSize allocatedSize = 0; ///< createBuffer 할 때, 만들어지는 버퍼의 크기
        VkDeviceSize alignment = 0; ///< 버퍼 정렬

        VkBufferUsageFlags usageFlags = 0; ///< 버퍼 사용 플래그
        VkMemoryPropertyFlags memoryPropertyFlags = 0; ///< 메모리 속성 플래그
        void* mapped = nullptr; ///< 매핑된 메모리 포인터

        VKResourceBinding resourceBinding;
    };
}

#endif // !INCLUDE_VKBUFFER_H_
