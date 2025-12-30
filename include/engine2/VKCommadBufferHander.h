#ifndef VK_COMMANDBUFFERHANDER_H_
#define VK_COMMANDBUFFERHANDER_H_

#include "common.h"

namespace vkengine {
    class VKCommandBufferHander
    {
    public:
        VKCommandBufferHander(VkDevice& device, VkCommandBuffer handle,VkCommandPool& commandPool, VkQueue& queue)
            : device(device), commandPool(commandPool), queue(queue), handle(handle) {}

        VKCommandBufferHander(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue, VkCommandBufferLevel level, cBool begin);

        ~VKCommandBufferHander()
        {
            cleanup();
        }

        // Move constructor
        VKCommandBufferHander(VKCommandBufferHander&& other) noexcept
            : device(other.device), commandPool(other.commandPool), queue(other.queue), handle(other.handle)
        {
            other.handle = VK_NULL_HANDLE; // 다른 객체의 핸들을 null로 설정하여 소유권 이전을 명확히 함
        }

        void cleanup() {
            if (handle != VK_NULL_HANDLE) {
                vkFreeCommandBuffers(device, commandPool, 1, &handle);
                handle = VK_NULL_HANDLE;
            }
        }

        VkCommandPool& getCommandPool() { return commandPool; }
        VkQueue& getQueue() { return queue; }
        VkCommandBuffer& getCommandBuffer() { return handle; }

        void submitAndWait();

    private:
        VkDevice& device;
        VkCommandPool& commandPool;
        VkQueue& queue;
        VkCommandBuffer handle{ VK_NULL_HANDLE };
    };
}

#endif // !VK_COMMANDBUFFERHANDER_H_
