#ifndef _VKCOMMANDBUFFER_H_
#define _VKCOMMANDBUFFER_H_

#include "common.h"
#include "struct.h"
#include "macros.h"
#include "helper.h"

namespace vkengine {
    struct VKCommandBufferHander {

        // �ٸ� �ڵ�� �̵� ������
        VKCommandBufferHander(VKCommandBufferHander&& other) noexcept
            : device(other.device), commandPool(other.commandPool), queue(other.queue), handle(other.handle) 
        {
            this->handle = VK_NULL_HANDLE;
        }

        VKCommandBufferHander(VkDevice& device, VkCommandBuffer& handle, VkCommandPool& pool,
            VkQueue& queue, VkCommandBufferLevel level);
        
        VKCommandBufferHander(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue,
            VkCommandBufferLevel level, bool begin);

        ~VKCommandBufferHander() {
            cleanup();
        }

        VkCommandBuffer& getHandle() { return handle; }
        VkQueue& getQueue() { return queue; }

        // Ŀ�ǵ� ���� ����� �����ϰ� ���� �� ���
        void submitAndWait();

    private:

        void cleanup();

        VkDevice& device;
        VkCommandPool& commandPool;
        VkQueue& queue;

        VkCommandBuffer handle = VK_NULL_HANDLE;

    };
}

#endif // !_VKCOMMANDBUFFER_H_
