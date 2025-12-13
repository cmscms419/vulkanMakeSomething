#include "VKCommadBufferHander.h"
#include "macros.h"
#include "helper.h"

namespace vkengine {
    VKCommandBufferHander::VKCommandBufferHander(VkDevice& device, VkCommandPool& commandPool, VkQueue& queue, VkCommandBufferLevel level, cBool begin)
        : device(device), commandPool(commandPool), queue(queue)
    {
        VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(commandPool, 1, level);

        _VK_CHECK_RESULT_(vkAllocateCommandBuffers(device, &allocInfo, &handle));

        if (begin) {
            VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            _VK_CHECK_RESULT_(vkBeginCommandBuffer(handle, &beginInfo));
        }

    }

    void VKCommandBufferHander::submitAndWait()
    {
        if (this->handle == VK_NULL_HANDLE) {
            return;
        }
        _VK_CHECK_RESULT_(vkEndCommandBuffer(this->handle));

        // 커맨드 버퍼 제출 정보 설정
        VkCommandBufferSubmitInfo cmdBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdBufferInfo.commandBuffer = this->handle;
        cmdBufferInfo.deviceMask = 0;

        // 새로운 VkSubmitInfo2 구조체 사용
        VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdBufferInfo;

        VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkFence fence;
        _VK_CHECK_RESULT_(vkCreateFence(this->device, &fenceCreateInfo, nullptr, &fence));

        _VK_CHECK_RESULT_(vkQueueSubmit2(this->queue, 1, &submitInfo, fence));

        _VK_CHECK_RESULT_(vkWaitForFences(this->device, 1, &fence, VK_TRUE, UINT64_MAX));

        vkDestroyFence(this->device, fence, nullptr);
    }
}