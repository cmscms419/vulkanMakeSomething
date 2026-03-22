#include "VKbuffer2.h"
#include "VKCommandBufferHander.h"
#include "helper.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{

    void VKBaseBuffer2::create(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data)
    {
        cleanup();

        this->descriptorCount = 1;
        this->usageFlags = usageFlags;
        this->memoryPropertyFlags = memoryPropertyFlags;
        this->bufferSize = size;
        this->offset = 0;

        helper::resource::createBuffer2(
            this->ctx.getDevice()->logicaldevice,
            this->ctx.getDevice()->physicalDevice,
            this->bufferSize,
            this->usageFlags,
            this->memoryPropertyFlags,
            this->buffer,
            this->memory,
            &this->allocatedSize,
            &this->alignment);

        // 버퍼 메모리를 매핑하여 CPU에서 접근할 수 있도록 합니다.
        // vkMapMemory 함수를 호출하여 메모리를 매핑하고,
        // 매핑된 메모리의 포인터를 mapped 멤버 변수에 저장합니다.
        // 버퍼 메모리를 매핑합니다.
        _VK_CHECK_RESULT_(vkMapMemory(ctx.getDevice()->logicaldevice, memory, this->offset, this->allocatedSize, 0, &mapped));

        // 매핑된 메모리에 데이터를 복사합니다.
        if (data != nullptr)
        {
            memcpy(this->mapped, data, this->allocatedSize);
            if ((this->memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            {
                this->flush();
            }
        }

        // 할당된 메모리를 버퍼와 바인딩하여 GPU에서 사용할 수 있게 한다.
        _VK_CHECK_RESULT_(vkBindBufferMemory(ctx.getDevice()->logicaldevice, this->buffer, this->memory, 0));
        this->update();
    }

    void VKBaseBuffer2::cleanup()
    {
        if (mapped)
        {
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

    void VKBaseBuffer2::createVertexBuffer(VkDeviceSize size, void *data)
    {
        this->create(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            size, data);
    }

    void VKBaseBuffer2::createIndexBuffer(VkDeviceSize size, void *data)
    {
        this->create(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            size, data);
    }

    void VKBaseBuffer2::createStagingBuffer(VkDeviceSize size, void *data)
    {
        this->create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            size, data);
    }

    void VKBaseBuffer2::createUniformBuffer(VkDeviceSize size, void *data)
    {
        this->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        this->create(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            size, data);
    }

    void VKBaseBuffer2::createModelVertexBuffer(VkDeviceSize size, void *data)
    {
        this->usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        this->memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        this->bufferSize = size;
        this->offset = 0;

        helper::resource::createBuffer2(
            this->ctx.getDevice()->logicaldevice,
            this->ctx.getDevice()->physicalDevice,
            this->bufferSize,
            this->usageFlags,
            this->memoryPropertyFlags,
            this->buffer,
            this->memory,
            &this->allocatedSize,
            &this->alignment);

        _VK_CHECK_RESULT_(vkBindBufferMemory(this->ctx.getDevice()->logicaldevice, this->buffer, this->memory, 0));
    }

    void VKBaseBuffer2::createModelIndexBuffer(VkDeviceSize size, void *data)
    {
        this->usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        this->memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        this->bufferSize = size;
        this->offset = 0;

        helper::resource::createBuffer2(
            this->ctx.getDevice()->logicaldevice,
            this->ctx.getDevice()->physicalDevice,
            this->bufferSize,
            this->usageFlags,
            this->memoryPropertyFlags,
            this->buffer,
            this->memory,
            &this->allocatedSize,
            &this->alignment);

        _VK_CHECK_RESULT_(vkBindBufferMemory(this->ctx.getDevice()->logicaldevice, this->buffer, this->memory, 0));
    }

    void VKBaseBuffer2::createStorageBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage)
    {
        this->descriptorCount = 1;
        this->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        VkDevice device = this->ctx.getDevice()->logicaldevice;

        this->bufferSize = size;

        // 버퍼 생성 정보를 담은 구조체를 초기화한다.
        VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; // 구조체 타입을 지정한다.
        bufferInfo.size = this->bufferSize;                      // 생성할 버퍼의 크기를 설정한다.
        bufferInfo.usage = additionalUsage;                      // 버퍼 사용 목적을 지정한다 (예: vertex, index 등).
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;      // 버퍼의 공유 모드를 독점으로 설정한다.

        // 지정된 정보를 바탕으로 버퍼를 생성
        _VK_CHECK_RESULT_(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

        // 생성된 버퍼에 필요한 메모리 요구사항 정보를 가져온다.
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
        uint32_t memoryTypeIndex = ctx.getMemoryTypeIndex(memRequirements.memoryTypeBits,
                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (memoryTypeIndex == uint32_t(-1))
        {
            memoryTypeIndex = ctx.getMemoryTypeIndex(memRequirements.memoryTypeBits,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            this->hostVisible = true;
        }

        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;
        _VK_CHECK_RESULT_(vkAllocateMemory(device, &allocInfo, nullptr, &memory));
        _VK_CHECK_RESULT_(vkBindBufferMemory(device, buffer, memory, 0));

        this->update();
    }

    void VKBaseBuffer2::updateData(const void *data, VkDeviceSize size, VkDeviceSize offset)
    {
        bool check = true;

        if (!mapped || !data)
        {
            check = false;
        }

        if (offset + size > this->bufferSize)
        {
            check = false;
        }

        if (!check)
            return;

        cUInt8_t *dst = static_cast<cUInt8_t *>(this->mapped) + offset;
        memcpy(dst, data, size);

        // 만약, 메모리가 일정하지 않으면, flush
        if ((this->memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            this->flush();
        }
    }

    void VKBaseBuffer2::copyData(const void *data, VkDeviceSize size, VkDeviceSize offset)
    {
        if (buffer == VK_NULL_HANDLE)
            return;

        if (hostVisible)
        {
            void *mappedData = map();
            if (mappedData)
            {
                memcpy(static_cast<char *>(mappedData) + offset, data, size);
            }
        }
        else
        {
            VkDevice device = this->ctx.getDevice()->logicaldevice;
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;
            VkDeviceSize stagingallCatedSize;
            VkDeviceSize stagingalignment;

            helper::resource::createBuffer2(
                device,
                this->ctx.getDevice()->physicalDevice,
                size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingMemory,
                &stagingallCatedSize,
                &stagingalignment);
            _VK_CHECK_RESULT_(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

            helper::resource::copyToDeviceMemory(device, data, stagingMemory, size);

            VKCommandBufferHander commandBuffer = this->ctx.createTransferCommandBufferHander(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = offset;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), stagingBuffer, this->buffer, 1, &copyRegion);

            commandBuffer.submitAndWait();

            // Cleanup staging resources
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingMemory, nullptr);
        }
    }

    void *VKBaseBuffer2::map()
    {
        if (!hostVisible || mapped != nullptr || buffer == VK_NULL_HANDLE)
        {
            return mapped;
        }

        const VkDevice device = this->ctx.getDevice()->logicaldevice;
        _VK_CHECK_RESULT_(vkMapMemory(device, memory, 0, bufferSize, 0, &mapped));

        return mapped;
    }

    void VKBaseBuffer2::unmap()
    {
        if (mapped != nullptr && buffer != VK_NULL_HANDLE)
        {
            const VkDevice device = this->ctx.getDevice()->logicaldevice;
            vkUnmapMemory(device, memory);
            mapped = nullptr;
        }
    }

    // CPU에서 작성한 메모리 변경사항을 GPU에 확실히 전달하는 역할
    void VKBaseBuffer2::flush() const
    {
        VkMappedMemoryRange mappedRange = {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
        mappedRange.memory = this->memory;
        mappedRange.offset = this->offset;
        mappedRange.size = this->allocatedSize;

        _VK_CHECK_RESULT_(vkFlushMappedMemoryRanges(ctx.getDevice()->logicaldevice, 1, &mappedRange));
    }

    void VKBaseBuffer2::updateBinding(VkDescriptorSetLayoutBinding &binding)
    {
        switch (this->descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            binding.descriptorType = this->descriptorType;
            binding.descriptorCount = this->descriptorCount;
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = this->stageFlags;
            break;
        default:
            EXIT_TO_LOGGER("Descriptor Type Buffer");
            break;
        }
    }

    void VKBaseBuffer2::updateWrite(VkWriteDescriptorSet &write)
    {
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstSet = VK_NULL_HANDLE; // Will be set by DescriptorSet::create()
        write.dstBinding = 0;          // Will be set by DescriptorSet::create()
        write.dstArrayElement = 0;
        write.descriptorType = this->descriptorType;
        write.descriptorCount = this->descriptorCount;
        write.pBufferInfo = &this->bufferInfo;
        write.pImageInfo = nullptr;
        write.pTexelBufferView = nullptr;
    }

    VKBaseBuffer2::VKBaseBuffer2(VKcontext &ctx) : ctx(ctx)
    {
        name = "Default";
        buffer = VK_NULL_HANDLE; //< Vulkan 버퍼 핸들
        memory = VK_NULL_HANDLE; ///< Vulkan 장치 메모리 핸들
        bufferInfo = {};         ///< Vulkan 디스크립터 버퍼 정보
        bufferSize = 0;          ///< 버퍼 크기
        offset = 0;              ///< 버퍼 간격
        allocatedSize = 0;       ///< createBuffer 할 때, 만들어지는 버퍼의 크기
        alignment = 0;           ///< 버퍼 정렬
        usageFlags = 0;          ///< 버퍼 사용 플래그
        memoryPropertyFlags = 0; ///< 메모리 속성 플래그
        mapped = nullptr;        ///< 매핑된 메모리 포인터
        hostVisible = false;
    }

    VKBaseBuffer2::VKBaseBuffer2(VKBaseBuffer2 &&other) noexcept
        : ctx(other.ctx),
          usageFlags(other.usageFlags), memoryPropertyFlags(other.memoryPropertyFlags)
    {
        // base class protected fields
        this->name = other.name;
        this->buffer = other.buffer;
        this->memory = other.memory;
        this->mapped = other.mapped;
        this->offset = other.offset;
        this->bufferSize = other.bufferSize;
        this->alignment = other.alignment;
        this->allocatedSize = other.allocatedSize;
        this->bufferInfo = other.bufferInfo;
        this->descriptorType = other.descriptorType;

        other.buffer = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
        other.mapped = nullptr;
        other.offset = 0;
        other.bufferSize = 0;
        other.alignment = 0;
        other.allocatedSize = 0;
    }
}
