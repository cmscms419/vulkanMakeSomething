#ifndef VK_COMMANDBUFFERHANDER_H_
#define VK_COMMANDBUFFERHANDER_H_

#include "common.h"

namespace vkengine {
    class VKCommandBufferHander
    {
    public:
        VKCommandBufferHander(VkDevice& device, VkCommandBuffer handle,VkCommandPool& commandPool, VkQueue& queue);
        VKCommandBufferHander(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue, VkCommandBufferLevel level, cBool begin);
        ~VKCommandBufferHander();

        // Move constructor
        VKCommandBufferHander(VKCommandBufferHander&& other) noexcept;

        void cleanup();

        VkCommandPool& getCommandPool();
        VkQueue& getQueue();
        VkCommandBuffer& getCommandBuffer();

        void submitAndWait();

    private:
        VkDevice& device;
        VkCommandPool& commandPool;
        VkQueue& queue;
        VkCommandBuffer handle{ VK_NULL_HANDLE };
    };
}

#endif // !VK_COMMANDBUFFERHANDER_H_
