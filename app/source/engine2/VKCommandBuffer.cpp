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
            
            // 명령 버퍼의 시작 정보를 설정합니다.
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
        
        // 커맨드 버퍼 제출 정보를 설정합니다.
        // 구조체 타입을 설정합니다.
        VkCommandBufferSubmitInfo cmdSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdSubmitInfo.commandBuffer = this->handle; // 제출할 커맨드 버퍼를 지정합니다.
        cmdSubmitInfo.pNext = nullptr; // 확장 정보를 위한 포인터를 설정합니다 (없으면 nullptr).
        cmdSubmitInfo.deviceMask = 0; // 멀티 GPU 환경에서 사용할 디바이스 마스크를 설정합니다 (단일 GPU에서는 0).

        // 제출 정보를 담은 구조체를 초기화합니다.
        // VkSubmitInfo보다 새로운 VkSubmitInfo2를 사용합니다.
        // 타임라인 세마포어 값 지원, 더 정확한 동기화 포인트 지정
        // 멀티 GPU 환경에서 디바이스 마스크 지정 가능, 더 정확한 제출 제어
        VkSubmitInfo2 submitInfo2{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 }; // 구조체 타입을 설정합니다.
        submitInfo2.pNext = nullptr; // 확장 정보를 위한 포인터를 설정합니다 (없으면 nullptr).
        submitInfo2.flags = 0; // 제출 플래그를 설정합니다 (기본값은 0).
        submitInfo2.waitSemaphoreInfoCount = 0; // 대기할 세마포어 정보의 개수를 설정합니다 (없으면 0).
        submitInfo2.pWaitSemaphoreInfos = nullptr; // 대기할 세마포어 정보 배열을 설정합니다 (없으면 nullptr).
        submitInfo2.commandBufferInfoCount = 1; // 제출할 커맨드 버퍼 정보의 개수를 설정합니다.
        submitInfo2.pCommandBufferInfos = &cmdSubmitInfo; // 제출할 커맨드 버퍼 정보 배열을 설정합니다.

        // Fence 생성(명령 버퍼 실행 완료를 알리기 위해)
        VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkFence fence;

        // fence 생성
        _VK_CHECK_RESULT_(vkCreateFence(device, &fenceInfo, nullptr, &fence));

        // 커맨드 버퍼를 큐에 제출
        _VK_CHECK_RESULT_(vkQueueSubmit2(this->queue, 1, &submitInfo2, fence));

        // 제출된 커맨드 버퍼가 완료될 때까지 대기
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