#include "BaseBuffer2.h"
#include "CommandBufferHandle.h"
#include "helper.h"
#include "log.h"


namespace vkengine
{

    void BaseBuffer2::create(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data)
    {
        cleanup();

        this->descriptorCount = 1;
        this->usageFlags = usageFlags;
        this->memoryPropertyFlags = memoryPropertyFlags;
        this->bufferSize = size;
        this->offset = 0;

        // 매핑은 HOST_VISIBLE 메모리에서만 할 수 있다. 이 값을 여기서 정해 두지
        // 않으면 copyData() 가 매핑 경로 대신 스테이징 경로로 빠지고, usage 에
        // TRANSFER_DST_BIT 가 없는 버퍼에서는 복사가 조용히 버려진다.
        // 이렇게 하고 문제가 생기면 추후 수정
        this->hostVisible = (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;

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

        // 할당된 메모리를 버퍼와 바인딩하여 GPU에서 사용할 수 있게 한다.
        // 아래 copyData() 가 이 버퍼를 복사 대상으로 쓰므로 바인딩이 먼저여야 한다.
        _VK_CHECK_RESULT_(vkBindBufferMemory(ctx.getDevice()->logicaldevice, this->buffer, this->memory, 0));

        if (this->hostVisible)
        {
            // 버퍼 메모리를 매핑하여 CPU 에서 접근할 수 있도록 한다.
            // 매핑된 포인터는 mapped 멤버에 보관하고, cleanup() 에서 해제한다.
            _VK_CHECK_RESULT_(vkMapMemory(ctx.getDevice()->logicaldevice, memory, this->offset, this->allocatedSize, 0, &mapped));

            // 매핑된 메모리에 데이터를 복사한다.
            if (data != nullptr)
            {
                memcpy(this->mapped, data, size);
                if ((this->memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
                {
                    this->flush();
                }
            }
        }
        else if (data != nullptr)
        {
            // DEVICE_LOCAL 메모리는 매핑할 수 없으니 스테이징으로 올린다.
            // 그러려면 usage 에 TRANSFER_DST_BIT 가 있어야 한다. 없으면
            // vkCmdCopyBuffer 가 거부되고 복사가 조용히 사라지므로 바로 멈춘다.
            if ((this->usageFlags & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0)
            {
                Log::exitToLogger("BaseBuffer2::create : device-local 버퍼에 초기 데이터를 넣으려면 "
                                  "usage 에 VK_BUFFER_USAGE_TRANSFER_DST_BIT 가 필요하다. "
                                  "usage 0x%08X", this->usageFlags);
            }

            this->copyData(data, size, 0);
        }

        this->update();
    }

    void BaseBuffer2::cleanup()
    {
        // 다시 만들 때 이전 버퍼의 상태가 남아 있으면 안 된다.
        this->hostVisible = false;

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

    void BaseBuffer2::createVertexBuffer(VkDeviceSize size, void *data)
    {
        this->create(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            size, data);
    }

    void BaseBuffer2::createIndexBuffer(VkDeviceSize size, void *data)
    {
        this->create(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            size, data);
    }

    void BaseBuffer2::createStagingBuffer(VkDeviceSize size, void *data)
    {
        this->create(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            size, data);
    }

    void BaseBuffer2::createUniformBuffer(VkDeviceSize size, void *data)
    {
        this->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        this->create(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            size, data);
    }

    // 모델용 버퍼는 DEVICE_LOCAL 이라 매핑이 안 된다. create() 가 그 경우를
    // 처리하므로 예전처럼 본문을 복붙할 필요가 없다. data 가 nullptr 이면
    // 버퍼만 만들고, 내용은 호출한 쪽이 copyData() 로 올린다.
    void BaseBuffer2::createModelVertexBuffer(VkDeviceSize size, void *data)
    {
        this->create(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            size, data);
    }

    void BaseBuffer2::createModelIndexBuffer(VkDeviceSize size, void *data)
    {
        this->create(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            size, data);
    }

    void BaseBuffer2::createStorageBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage)
    {
        // 이미 버퍼를 들고 있으면 먼저 버린다. 이게 없으면 다시 만들 때마다
        // 앞의 VkBuffer / VkDeviceMemory 가 그대로 샌다.
        cleanup();

        this->descriptorCount = 1;
        this->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        VkDevice device = this->ctx.getDevice()->logicaldevice;

        this->usageFlags = additionalUsage;
        this->bufferSize = size;
        this->offset = 0;

        // 버퍼 생성 정보를 담은 구조체를 초기화한다.
        VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; // 구조체 타입을 지정한다.
        bufferInfo.size = this->bufferSize;                      // 생성할 버퍼의 크기를 설정한다.
        bufferInfo.usage = this->usageFlags;                     // 버퍼 사용 목적을 지정한다 (예: vertex, index 등).
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;      // 버퍼의 공유 모드를 독점으로 설정한다.

        // 지정된 정보를 바탕으로 버퍼를 생성
        _VK_CHECK_RESULT_(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

        // 생성된 버퍼에 필요한 메모리 요구사항 정보를 가져온다.
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        // flush() / updateData() 가 이 두 값을 본다. 안 채우면 size 0 으로 들어간다.
        this->allocatedSize = memRequirements.size;
        this->alignment = memRequirements.alignment;

        this->memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t memoryTypeIndex = ctx.getMemoryTypeIndex(memRequirements.memoryTypeBits,
                                                          this->memoryPropertyFlags);
        if (memoryTypeIndex == uint32_t(-1))
        {
            this->memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            memoryTypeIndex = ctx.getMemoryTypeIndex(memRequirements.memoryTypeBits,
                                                     this->memoryPropertyFlags);
            this->hostVisible = true;
        }

        VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;
        _VK_CHECK_RESULT_(vkAllocateMemory(device, &allocInfo, nullptr, &memory));
        _VK_CHECK_RESULT_(vkBindBufferMemory(device, buffer, memory, 0));

        this->update();
    }

    void BaseBuffer2::createDynamicStorageBuffer(VkDeviceSize size, VkBufferUsageFlags additionalUsage)
    {
        this->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        // hostVisible / descriptorCount / update() 는 create() 가 처리한다.
        this->create(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | additionalUsage,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     size, nullptr);
    }

    void BaseBuffer2::updateData(const void *data, VkDeviceSize size, VkDeviceSize offset)
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

    void BaseBuffer2::copyData(const void *data, VkDeviceSize size, VkDeviceSize offset)
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

            CommandBufferHandle commandBuffer = this->ctx.createTransferCommandBufferHandle(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

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

    void *BaseBuffer2::map()
    {
        if (!hostVisible || mapped != nullptr || buffer == VK_NULL_HANDLE)
        {
            return mapped;
        }

        const VkDevice device = this->ctx.getDevice()->logicaldevice;
        _VK_CHECK_RESULT_(vkMapMemory(device, memory, 0, bufferSize, 0, &mapped));

        return mapped;
    }

    void BaseBuffer2::unmap()
    {
        if (mapped != nullptr && buffer != VK_NULL_HANDLE)
        {
            const VkDevice device = this->ctx.getDevice()->logicaldevice;
            vkUnmapMemory(device, memory);
            mapped = nullptr;
        }
    }

    // CPU에서 작성한 메모리 변경사항을 GPU에 확실히 전달하는 역할
    void BaseBuffer2::flush() const
    {
        VkMappedMemoryRange mappedRange = {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
        mappedRange.memory = this->memory;
        mappedRange.offset = this->offset;
        mappedRange.size = this->allocatedSize;

        _VK_CHECK_RESULT_(vkFlushMappedMemoryRanges(ctx.getDevice()->logicaldevice, 1, &mappedRange));
    }

    BaseBuffer2::BaseBuffer2(Context &ctx) : ctx(ctx)
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

    BaseBuffer2::BaseBuffer2(BaseBuffer2 &&other) noexcept
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
        this->descriptorCount = other.descriptorCount;

        other.buffer = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
        other.mapped = nullptr;
        other.offset = 0;
        other.bufferSize = 0;
        other.alignment = 0;
        other.allocatedSize = 0;
    }
}
