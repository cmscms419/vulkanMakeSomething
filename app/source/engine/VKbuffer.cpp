#include "VKbuffer.h"
#include "helper.h"

using namespace vkengine::helper;
namespace vkengine {

    void VkBaseBuffer::cleanup()
    {
        if (buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device, buffer, nullptr);
        }
        if (memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, memory, nullptr);
        }
    }

    // 버퍼 메모리를 매핑하여 CPU에서 접근할 수 있도록 합니다. 
    // vkMapMemory 함수를 호출하여 메모리를 매핑하고, 
    // 매핑된 메모리의 포인터를 mapped 멤버 변수에 저장합니다.
    VkResult VkBaseBuffer::mapToMeBuffer(VkDeviceSize size, VkDeviceSize offset)
    {
        // 버퍼 메모리를 매핑합니다.
        return vkMapMemory(device, memory, offset, size, 0, &mapped);
    }

    void VkBaseBuffer::copyToMeBuffer(void* data, VkDeviceSize size)
    {
        // 매핑된 메모리에 데이터를 복사합니다.
        memcpy(this->mapped, data, size);

        // 버퍼 메모리의 매핑을 해제합니다.
        vkUnmapMemory(device, memory);
    }

    void VkBaseBuffer::createBuffer(VkPhysicalDevice physicalDevice)
    {
        vkengine::helper::createBuffer(
            this->device,
            physicalDevice,
            this->size,
            this->usageFlags,
            this->memoryPropertyFlags,
            this->buffer,
            this->memory);
    }
}
