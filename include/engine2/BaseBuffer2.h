#ifndef INCLUDE_VKBUFFER_H_
#define INCLUDE_VKBUFFER_H_

#include "common.h"

#include "Context.h"
#include "ShaderResource.h"

namespace vkengine
{

    class BaseBuffer2 : public BufferShaderResource
    {
    public:
        BaseBuffer2(Context &ctx);
        BaseBuffer2(BaseBuffer2 &&) noexcept;

        BaseBuffer2(const BaseBuffer2 &) = delete;
        BaseBuffer2 &operator=(const BaseBuffer2 &) = delete;
        BaseBuffer2 &operator=(BaseBuffer2 &&) = delete;

        ~BaseBuffer2()
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

        VkBuffer &getBuffer() { return this->buffer; }
        void *getMapped() { return this->mapped; }

    private:
        void create(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
                    VkDeviceSize size, void *data);
        Context &ctx;
        cBool hostVisible;

        VkBufferUsageFlags usageFlags;             ///< 버퍼 사용 플래그
        VkMemoryPropertyFlags memoryPropertyFlags; ///< 메모리 속성 플래그
    };
}

#endif // !INCLUDE_VKBUFFER_H_
