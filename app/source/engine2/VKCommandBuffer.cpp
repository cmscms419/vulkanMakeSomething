#include "VKCommandBuffer.h"

namespace vkengine {
    VKCommandBufferHander::VKCommandBufferHander(VkDevice& device, VkCommandBuffer& handle, VkCommandPool& pool, VkQueue& queue, VkCommandBufferLevel level) :
        device(device), commandPool(pool), queue(queue), handle(handle)
    {
    }
    VKCommandBufferHander::VKCommandBufferHander(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue, VkCommandBufferLevel level, bool begin)
        : device(device), commandPool(commandPool), queue(queue)
    {
        VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(this->commandPool, 1, level);

        _VK_CHECK_RESULT_(vkAllocateCommandBuffers(device, &allocInfo, &handle));

        if (begin) {
            VkCommandBufferBeginInfo beginInfo{};
            
            // ��� ������ ���� ������ �����մϴ�.
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            _VK_CHECK_RESULT_(vkBeginCommandBuffer(handle, &beginInfo));

        }
    }
    void VKCommandBufferHander::submitAndWait()
    {
        if (this->handle == VK_NULL_HANDLE) {
            _EXIT_WITH_MESSAGE_("Command buffer handle is null, cannot submit.");
        }

        _VK_CHECK_RESULT_(vkEndCommandBuffer(this->handle));
        
        // Ŀ�ǵ� ���� ���� ������ �����մϴ�.
        // ����ü Ÿ���� �����մϴ�.
        VkCommandBufferSubmitInfo cmdSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdSubmitInfo.commandBuffer = this->handle; // ������ Ŀ�ǵ� ���۸� �����մϴ�.
        cmdSubmitInfo.pNext = nullptr; // Ȯ�� ������ ���� �����͸� �����մϴ� (������ nullptr).
        cmdSubmitInfo.deviceMask = 0; // ��Ƽ GPU ȯ�濡�� ����� ����̽� ����ũ�� �����մϴ� (���� GPU������ 0).

        // ���� ������ ���� ����ü�� �ʱ�ȭ�մϴ�.
        // VkSubmitInfo���� ���ο� VkSubmitInfo2�� ����մϴ�.
        // Ÿ�Ӷ��� �������� �� ����, �� ��Ȯ�� ����ȭ ����Ʈ ����
        // ��Ƽ GPU ȯ�濡�� ����̽� ����ũ ���� ����, �� ��Ȯ�� ���� ����
        VkSubmitInfo2 submitInfo2{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 }; // ����ü Ÿ���� �����մϴ�.
        submitInfo2.pNext = nullptr; // Ȯ�� ������ ���� �����͸� �����մϴ� (������ nullptr).
        submitInfo2.flags = 0; // ���� �÷��׸� �����մϴ� (�⺻���� 0).
        submitInfo2.waitSemaphoreInfoCount = 0; // ����� �������� ������ ������ �����մϴ� (������ 0).
        submitInfo2.pWaitSemaphoreInfos = nullptr; // ����� �������� ���� �迭�� �����մϴ� (������ nullptr).
        submitInfo2.commandBufferInfoCount = 1; // ������ Ŀ�ǵ� ���� ������ ������ �����մϴ�.
        submitInfo2.pCommandBufferInfos = &cmdSubmitInfo; // ������ Ŀ�ǵ� ���� ���� �迭�� �����մϴ�.

        // Fence ����(��� ���� ���� �ϷḦ �˸��� ����)
        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkFence fence;

        // fence ����
        _VK_CHECK_RESULT_(vkCreateFence(device, &fenceInfo, nullptr, &fence));

        // Ŀ�ǵ� ���۸� ť�� ����
        _VK_CHECK_RESULT_(vkQueueSubmit2(this->queue, 1, &submitInfo2, fence));

        // ����� Ŀ�ǵ� ���۰� �Ϸ�� ������ ���
        _VK_CHECK_RESULT_(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));
        vkDestroyFence(device, fence, nullptr);
    }

    void VKCommandBufferHander::cleanup() {
        if (handle != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(device, commandPool, 1, &handle);
            handle = VK_NULL_HANDLE;
        }
    }
}