#include "helper.h"
#include "VKbuffer2.h"

using namespace vkengine::helper;
namespace vkengine {

    void VKBaseBuffer2::cleanup()
    {
        if (mapped) {
            vkUnmapMemory(ctx.getDevice()->logicaldevice, memory);
            mapped = nullptr;
        }
        if (buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(ctx.getDevice()->logicaldevice, buffer, nullptr);
            buffer = VK_NULL_HANDLE;
        }
        if (memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(ctx.getDevice()->logicaldevice, memory, nullptr);
            memory = VK_NULL_HANDLE;
        }
    }

    void VKBaseBuffer2::create(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
    {
        cleanup();

        this->usageFlags = usageFlags;
        this->memoryPropertyFlags = memoryPropertyFlags;
        this->size = size;
        this->offset = 0;

        vkengine::helper::createBuffer2(
            this->ctx.getDevice()->logicaldevice,
            this->ctx.getDevice()->physicalDevice,
            this->size,
            this->usageFlags,
            this->memoryPropertyFlags,
            this->buffer,
            this->memory,
            &this->allocatedSize,
            &this->alignment
        );
        // 버퍼 메모리를 매핑하여 CPU에서 접근할 수 있도록 합니다. 
        // vkMapMemory 함수를 호출하여 메모리를 매핑하고, 
        // 매핑된 메모리의 포인터를 mapped 멤버 변수에 저장합니다.
        // 버퍼 메모리를 매핑합니다.
        _VK_CHECK_RESULT_(vkMapMemory(ctx.getDevice()->logicaldevice, memory, this->offset, this->allocatedSize, 0, &mapped));

        // 매핑된 메모리에 데이터를 복사합니다.
        if (data != nullptr) {
            memcpy(this->mapped, data, this->allocatedSize);
            if ((this->memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            {
                this->flush();
            }
        }

        // 할당된 메모리를 버퍼와 바인딩하여 GPU에서 사용할 수 있게 한다.
        _VK_CHECK_RESULT_(vkBindBufferMemory(ctx.getDevice()->logicaldevice, this->buffer, this->memory, 0));

    }

    void VKBaseBuffer2::createVertexBuffer(VkDeviceSize size, void* data)
    {
        /*this->create(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            size,
            data
        );*/

        this->create(
           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            size, data);
    }

    void VKBaseBuffer2::createIndexBuffer(VkDeviceSize size, void* data)
    {
        /*this->create(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            size,
            data
        );*/

        this->create(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            size, data);
    }

    void VKBaseBuffer2::createStagingBuffer(VkDeviceSize size, void* data)
    {
        this->create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            size, data);
    }

    void VKBaseBuffer2::createUniformBuffer(VkDeviceSize size, void* data)
    {
        this->create(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            size, data);

        resourceBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        resourceBinding.buffer = this->buffer;
        resourceBinding.bufferSize = this->size;
        resourceBinding.descriptorCount = 1;
        resourceBinding.update();
    }

    void VKBaseBuffer2::createModelVertexBuffer(VkDeviceSize size, void* data)
    {
        this->usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        this->memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        this->size = size;
        this->offset = 0;

        vkengine::helper::createBuffer2(
            this->ctx.getDevice()->logicaldevice,
            this->ctx.getDevice()->physicalDevice,
            this->size,
            this->usageFlags,
            this->memoryPropertyFlags,
            this->buffer,
            this->memory,
            &this->allocatedSize,
            &this->alignment
        );

        _VK_CHECK_RESULT_(vkBindBufferMemory(this->ctx.getDevice()->logicaldevice, this->buffer, this->memory, 0));
    }

    void VKBaseBuffer2::createModeIndexBuffer(VkDeviceSize size, void* data)
    {
        this->usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        this->memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        this->size = size;
        this->offset = 0;

        vkengine::helper::createBuffer2(
            this->ctx.getDevice()->logicaldevice,
            this->ctx.getDevice()->physicalDevice,
            this->size,
            this->usageFlags,
            this->memoryPropertyFlags,
            this->buffer,
            this->memory,
            &this->allocatedSize,
            &this->alignment
        );

        _VK_CHECK_RESULT_(vkBindBufferMemory(this->ctx.getDevice()->logicaldevice, this->buffer, this->memory, 0));
    }

    void VKBaseBuffer2::updateData(const void* data, VkDeviceSize size, VkDeviceSize offset)
    {
        bool check = true;

        if (!mapped || !data) {
            check = false;
        }

        if (offset + size > this->size)
        {
            check = false;
        }

        if (!check) return;

        cUInt8_t* dst = static_cast<cUInt8_t*>(this->mapped) + offset;
        memcpy(dst, data, size);

        // 만약, 메모리가 일정하지 않으면, flush
        if ((this->memoryPropertyFlags& VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            this->flush();
        }
    }

    // CPU에서 작성한 메모리 변경사항을 GPU에 확실히 전달하는 역할
    void VKBaseBuffer2::flush() const
    {
        VkMappedMemoryRange mappedRange = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
        mappedRange.memory = this->memory;
        mappedRange.offset = this->offset;
        mappedRange.size = this->allocatedSize;

        _VK_CHECK_RESULT_(vkFlushMappedMemoryRanges(ctx.getDevice()->logicaldevice, 1, &mappedRange));
    }

    VKBaseBuffer2::VKBaseBuffer2(VKcontext& ctx) : ctx(ctx)
    {

    }

    VKBaseBuffer2::VKBaseBuffer2(VKBaseBuffer2&& other) noexcept
        : ctx(other.ctx), name(std::move(other.name)), buffer(other.buffer),
        memory(other.memory), descriptor(other.descriptor), size(other.size),
        offset(other.offset), alignment(other.alignment), usageFlags(other.usageFlags),
        memoryPropertyFlags(other.memoryPropertyFlags), resourceBinding(std::move(other.resourceBinding)),
        allocatedSize(other.allocatedSize)
    {
        other.buffer = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
        other.mapped = VK_NULL_HANDLE;
        other.size = 0;
        other.alignment = 0;
        other.allocatedSize = 0;
    }
}
