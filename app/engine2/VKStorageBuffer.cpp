#include "VKStorageBuffer.h"

#include "macros.h"

#include "helper.h"

namespace vkengine
{
    VKStorgeBuffer::VKStorgeBuffer(VKcontext &ctx) : ctx(ctx)
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        void *mapped = nullptr;
        bool hostVisible = false;
    }

    VKStorgeBuffer::~VKStorgeBuffer()
    {
        this->cleanup();
    }

    void VKStorgeBuffer::create(VkDeviceSize size, VkBufferUsageFlags additionalUsage)
    {
    }

    void *VKStorgeBuffer::map()
    {
        if (!hostVisible || mapped != nullptr || buffer == VK_NULL_HANDLE)
        {
            return mapped;
        }

        const VkDevice device = this->ctx.getDevice()->logicaldevice;
        _VK_CHECK_RESULT_(vkMapMemory(device, memory, 0, size, 0, &mapped));
        return mapped;
    }

    void VKStorgeBuffer::unmap()
    {
        if (mapped != nullptr && buffer != VK_NULL_HANDLE)
        {
            const VkDevice device = this->ctx.getDevice()->logicaldevice;
            vkUnmapMemory(device, memory);
            mapped = nullptr;
        }
    }

    void VKStorgeBuffer::copyData(const void *data, VkDeviceSize size, VkDeviceSize offset)
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
#if 0
            // Use staging buffer for device-local memory
            const VkDevice device = this->ctx.getDevice()->logicaldevice;

            // Create staging buffer
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;

            VkBufferCreateInfo stagingBufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            stagingBufferInfo.size = size;
            stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            _VK_CHECK_RESULT_(vkCreateBuffer(device, &stagingBufferInfo, nullptr, &stagingBuffer));

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = ctx.getMemoryTypeIndex(
                memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            _VK_CHECK_RESULT_(vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory));
            _VK_CHECK_RESULT_(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

            // Copy data to staging buffer
            void *stagingMapped;
            _VK_CHECK_RESULT_(vkMapMemory(device, stagingMemory, 0, size, 0, &stagingMapped));
            memcpy(stagingMapped, data, size);
            vkUnmapMemory(device, stagingMemory);

            // Copy from staging buffer to storage buffer
            VKCommandBufferHander commandBuffer = ctx.createTransferCommandBufferHander(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = offset;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), stagingBuffer, buffer, 1, &copyRegion);

            commandBuffer.submitAndWait();

            // Cleanup staging resources
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingMemory, nullptr);
#else
            cleanup();

            // Create staging buffer
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;

            VkDevice logicaldevice = this->ctx.getDevice()->logicaldevice;
            VkPhysicalDevice physicaldevice = this->ctx.getDevice()->physicalDevice;
            VkDeviceSize *allocatedSize;
            VkDeviceSize *alignment;

            helper::resource::createBuffer2(
                logicaldevice,
                physicaldevice,
                size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingMemory,
                allocatedSize,
                alignment);

            helper::resource::copyToDeviceMemory(logicaldevice, data, stagingMemory, size);

            // Copy from staging buffer to storage buffer
            VKCommandBufferHander commandBuffer = ctx.createTransferCommandBufferHander(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = offset;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), stagingBuffer, buffer, 1, &copyRegion);

            commandBuffer.submitAndWait();

            // Cleanup staging resources
            vkDestroyBuffer(logicaldevice, stagingBuffer, nullptr);
            vkFreeMemory(logicaldevice, stagingMemory, nullptr);

#endif
        }
    }

    VkDescriptorBufferInfo VKStorgeBuffer::getDescriptorInfo() const
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = size;
        return bufferInfo;
    }

    void VKStorgeBuffer::cleanup()
    {
        const VkDevice device = this->ctx.getDevice()->logicaldevice;

        unmap();

        if (buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device, buffer, nullptr);
            buffer = VK_NULL_HANDLE;
        }

        if (memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, memory, nullptr);
            memory = VK_NULL_HANDLE;
        }

        size = 0;
        hostVisible = false;
    }
}