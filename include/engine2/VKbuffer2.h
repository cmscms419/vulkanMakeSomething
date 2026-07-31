#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "common.h"

#include "VKContext.h"
#include "VKShaderResource.h"

namespace vkengine
{

    class VKBaseBuffer2 : public VKBufferShaderResource
    {
    public:
        VKBaseBuffer2(VKcontext &ctx);
        VKBaseBuffer2(VKBaseBuffer2 &&) noexcept;

        VKBaseBuffer2(const VKBaseBuffer2 &) = delete;
        VKBaseBuffer2 &operator=(const VKBaseBuffer2 &) = delete;
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
        void createModelIndexBuffer(VkDeviceSize size, void *data);
        void createStorageBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage = 0);
        void createDynamicStorageBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage = 0);

        void updateData(const void *data, VkDeviceSize size, VkDeviceSize offset);
        void copyData(const void *data, VkDeviceSize size, VkDeviceSize offset = 0);
        void flush() const;
        void *map();
        void unmap();

        VkBuffer &Buffer() { return this->buffer; }
        void *Mapped() { return this->mapped; }

    private:
        void create(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
                    VkDeviceSize size, void *data);
        VKcontext &ctx;
        cBool hostVisible;

        VkBufferUsageFlags usageFlags;             ///< 버퍼 사용 플래그
        VkMemoryPropertyFlags memoryPropertyFlags; ///< 메모리 속성 플래그
    };
}

#endif // !INCLUDE_VKBUFFER_H_
