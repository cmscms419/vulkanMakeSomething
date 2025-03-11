#include "VKbuffer.h"
#include "helper.h"

using namespace vkengine::helper;

void VkBufferObject::cleanup()
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

// �Լ��� ���� �޸𸮸� �����Ͽ� CPU���� ������ �� �ֵ��� �մϴ�. 
// vkMapMemory �Լ��� ȣ���Ͽ� �޸𸮸� �����ϰ�, 
// ���ε� �޸��� �����͸� mapped ��� ������ �����մϴ�.
VkResult VkBufferObject::mapToMeBuffer(VkDeviceSize size, VkDeviceSize offset)
{
    // ���� �޸𸮸� �����մϴ�.
    return vkMapMemory(device, memory, offset, size, 0, &mapped);
}

void VkBufferObject::copyToMeBuffer(void* data, VkDeviceSize size)
{
    // ���ε� �޸𸮿� �����͸� �����մϴ�.
    memcpy(this->mapped, data, size);

    // ���� �޸��� ������ �����մϴ�.
    vkUnmapMemory(device, memory);
}

void VkBufferObject::createBuffer(VkPhysicalDevice physicalDevice)
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
