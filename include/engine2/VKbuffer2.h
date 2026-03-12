#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "common.h"

#include "VKContext.h"
#include "VKShaderResource.h"

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

        void cleanup() override;

        void createVertexBuffer(VkDeviceSize size, void *data);
        void createIndexBuffer(VkDeviceSize size, void *data);
        void createStagingBuffer(VkDeviceSize size, void *data);
        void createUniformBuffer(VkDeviceSize size, void *data);
        void createModelVertexBuffer(VkDeviceSize size, void *data);
        void createModeIndexBuffer(VkDeviceSize size, void *data);

        void updateData(const void *data, VkDeviceSize size, VkDeviceSize offset);
        void flush() const;
        
        auto Buffer() -> VkBuffer &
        {
            return this->buffer;
        }

        auto Mapped() const -> void *
        {
            return this->mapped;
        }

    private:
        void create(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
                    VkDeviceSize size, void *data);

        cString name;
        VKcontext &ctx;

        VkDeviceSize size;                   ///< 버퍼 크기
        VkDeviceSize offset;                 ///< 버퍼 간격
        VkDeviceSize allocatedSize;          ///< createBuffer 할 때, 만들어지는 버퍼의 크기
        VkDeviceSize alignment;              ///< 버퍼 정렬

        VkBufferUsageFlags usageFlags;             ///< 버퍼 사용 플래그
        VkMemoryPropertyFlags memoryPropertyFlags; ///< 메모리 속성 플래그
    };
}

#endif // !INCLUDE_VKBUFFER_H_
