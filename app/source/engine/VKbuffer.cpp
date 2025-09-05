#include "helper.h"
#include "VKbuffer.h"

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

    // ���� �޸𸮸� �����Ͽ� CPU���� ������ �� �ֵ��� �մϴ�. 
    // vkMapMemory �Լ��� ȣ���Ͽ� �޸𸮸� �����ϰ�, 
    // ���ε� �޸��� �����͸� mapped ��� ������ �����մϴ�.
    VkResult VkBaseBuffer::mapToMeBuffer(VkDeviceSize size, VkDeviceSize offset)
    {
        // ���� �޸𸮸� �����մϴ�.
        return vkMapMemory(device, memory, offset, size, 0, &mapped);
    }

    void VkBaseBuffer::copyToMeBuffer(void* data, VkDeviceSize size)
    {
        // ���ε� �޸𸮿� �����͸� �����մϴ�.
        memcpy(this->mapped, data, size);

        // ���� �޸��� ������ �����մϴ�.
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
