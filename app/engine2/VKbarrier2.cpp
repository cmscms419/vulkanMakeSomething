#include "VKbarrier2.h"

using namespace vkengine::Log;

namespace vkengine {

    VKBarrierHelper::VKBarrierHelper() {}

    VKBarrierHelper::VKBarrierHelper(const VkImage& image) : image(image) {}

    VkCommandBuffer VKBarrierHelper::beginSingleTimeCommands2(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level, cBool oneTime)
    {
        VkCommandBuffer buffer{ VK_NULL_HANDLE }; // 커맨드 버퍼 핸들

        VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(commandPool, 1, level);

        _VK_CHECK_RESULT_(vkAllocateCommandBuffers(device, &allocInfo, &buffer));  // 커맨드 버퍼를 할당합니다.

        VkCommandBufferBeginInfo beginInfo{};
        // VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO : 명령 버퍼의 시작 정보를 설정합니다.
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 커맨드 버퍼를 한 번만 사용하려는 경우 사용합니다.
        if (oneTime)
        {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        
        // 커맨드 버퍼를 시작합니다.
        _VK_CHECK_RESULT_(vkBeginCommandBuffer(buffer, &beginInfo));

        return buffer;
    }

    void VKBarrierHelper::endSingleTimeCommands2(VkDevice device, VkCommandPool commandPool, VkQueue Queue, VkCommandBuffer commandBuffer)
    {
        _VK_CHECK_RESULT_(vkEndCommandBuffer(commandBuffer));

        // 커맨드 버퍼 제출 정보를 설정합니다.
        // 구조체 타입을 설정합니다.
        VkCommandBufferSubmitInfo cmdSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdSubmitInfo.commandBuffer = commandBuffer; // 제출할 커맨드 버퍼를 지정합니다.
        cmdSubmitInfo.pNext = nullptr;  // 확장 정보를 위한 포인터를 설정합니다 (없으면 nullptr).
        cmdSubmitInfo.deviceMask = 0;   // 멀티 GPU 환경에서 사용할 디바이스 마스크를 설정합니다 (단일 GPU에서는 0).

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
        _VK_CHECK_RESULT_(vkQueueSubmit2(Queue, 1, &submitInfo2, fence));

        // 제출된 커맨드 버퍼가 완료될 때까지 대기
        _VK_CHECK_RESULT_(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));
        vkDestroyFence(device, fence, nullptr);

        // 커맨드 버퍼를 해제합니다.
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void VKBarrierHelper::transitionImageLayout2(
        VkCommandBuffer commandBuffer,
        VkImageLayout newLayout,
        VkAccessFlags2 newAccess,
        VkPipelineStageFlags2 newStage,
        cUint32_t baseMipLevel,
        cUint32_t levelCount,
        cUint32_t baseArrayLayer,
        cUint32_t layerCount)
    {
        if (this->image == VK_NULL_HANDLE)
        {
            return;
        }

        cUint32_t actualLevelCount =
            (levelCount == VK_REMAINING_MIP_LEVELS) ? (mipLevels - baseMipLevel) : levelCount;
        cUint32_t actualLayerCount = (layerCount == VK_REMAINING_ARRAY_LAYERS)
            ? (arrayLayers - baseArrayLayer) : layerCount;

        if (currentLayout == newLayout && currentAccess == newAccess && baseMipLevel == 0 &&
            actualLevelCount == mipLevels && baseArrayLayer == 0 &&
            actualLayerCount == arrayLayers) {
            return;
        }

        // 이미지 메모리 배리어 구조체를 초기화하여 레이아웃 전환 및 접근 권한 변경을 정의합니다.
        // 기본적으로 색상 정보를 대상으로 하지만, 이후 조건에 따라 수정됩니다.
        VkImageMemoryBarrier2 barrier2{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };

        barrier2.image = this->image; // 전환할 이미지 핸들 설정
        barrier2.srcStageMask = (currentStage != VK_PIPELINE_STAGE_2_NONE)  // 파이프라인 단계 설정
            ? currentStage : VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        barrier2.dstStageMask = newStage;                                  // 파이프라인 단계에 적용될 flag
        barrier2.srcAccessMask = currentAccess;                             // 원본의 마스크를 정의
        barrier2.dstAccessMask = newAccess;                                 // 전환 될 마스크를 정의
        barrier2.oldLayout = currentLayout;                                 // 전환 전 이미지 레이아웃
        barrier2.newLayout = newLayout;                                     // 전환할 이미지 레이아웃

        // 기본적으로 색상 정보를 대상으로 하지만, 이후 조건에 따라 수정됩니다.
        barrier2.subresourceRange = {
            getAspectFlagsFromLayout(this->format),     // 이미지의 어떤 부분(색상, 깊이 등)에 영향을 미칠지 정의
            baseMipLevel,                               // 첫 번째 미프맵 레벨부터 시작
            actualLevelCount,                           // 적용할 미프맵의 수     
            baseArrayLayer,                             // 첫 번째 배열 레이어부터 시작
            actualLayerCount                            // 배열 내 레이어 개수 
        };

        barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 소스 큐 패밀리 인덱스 무시
        barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 대상 큐 패밀리 인덱스 무시

        // 파이프라인 배리어를 추가하여 레이아웃 전환 명령을 기록합니다.
        VkDependencyInfo depInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &barrier2;

        // 단일 명령 버퍼를 제출하고, 큐가 해당 작업을 완료할 때까지 대기합니다.
        vkCmdPipelineBarrier2(commandBuffer, &depInfo);

        // 현재 상태를 업데이트합니다.
        if (baseMipLevel == 0 && actualLevelCount == mipLevels &&
            baseArrayLayer == 0 && actualLayerCount == arrayLayers) {
            currentLayout = newLayout;
            currentAccess = newAccess;
            currentStage = newStage;
        }
    }

}