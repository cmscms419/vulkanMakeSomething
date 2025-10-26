#ifndef _VK_BARRIER_H_
#define _VK_BARRIER_H_

#include "common.h"
#include "struct.h"
#include "helper.h"
#include "log.h"

namespace vkengine {

    class VKBarrierHelper
    {
    public:

        VKBarrierHelper() {}

        // Move constructor
        VKBarrierHelper(VKBarrierHelper&& other) noexcept
            : mipLevels(other.mipLevels),
            arrayLayers(other.arrayLayers), currentLayout(other.currentLayout),
            currentAccess(other.currentAccess), currentStage(other.currentStage)
        {
            // Reset the moved-from object to a safe state
            other.mipLevels = 1;
            other.arrayLayers = 1;
            other.currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            other.currentAccess = VK_ACCESS_2_NONE;
            other.currentStage = VK_PIPELINE_STAGE_2_NONE;
        }

        // Move assignment operator
        VKBarrierHelper& operator=(VKBarrierHelper&& other) noexcept
        {
            if (this != &other) {
                mipLevels = other.mipLevels;
                arrayLayers = other.arrayLayers;
                currentLayout = other.currentLayout;
                currentAccess = other.currentAccess;
                currentStage = other.currentStage;

                // Reset the moved-from object to a safe state
                other.mipLevels = 1;
                other.arrayLayers = 1;
                other.currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                other.currentAccess = VK_ACCESS_2_NONE;
                other.currentStage = VK_PIPELINE_STAGE_2_NONE;
            }
            return *this;
        }

        void update(cUint32_t mipLevels, cUint32_t arrayLayers)
        {
            this->mipLevels = mipLevels;
            this->arrayLayers = arrayLayers;
        }


        // 시작하려는 명령버퍼를 생성하는 함수 (레벨 지정 가능. 임이로 지정한 VkCommandBuffer에 지정 가능)
        VkCommandBuffer beginSingleTimeCommands2(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level);

        // 명령버퍼를 종료하는 함수 (VkSubmitInfo2 사용)
        void endSingleTimeCommands2(VkDevice device, VkCommandPool commandPool, VkQueue Queue, VkCommandBuffer commandBuffer);

        /**

         * @brief 이미지 레이아웃 전환을 위한 고급 함수 (Vulkan 1.3+ VkImageMemoryBarrier2 사용)
         *
         * 이 함수는 Vulkan 이미지의 레이아웃을 안전하게 전환하기 위해 파이프라인 배리어를 설정합니다.
         * VkImageMemoryBarrier2를 사용하여 더 정밀한 동기화 제어와 확장된 파이프라인 스테이지 지원을 제공합니다.
         *
         * @param commandBuffer 이미지 레이아웃 전환 명령을 기록할 커맨드 버퍼
         * @param image 레이아웃을 전환할 VkImage 핸들 (VK_NULL_HANDLE인 경우 함수는 즉시 반환)
         * @param Queue 사용되지 않는 매개변수 (향후 호환성을 위해 유지)
         * @param format 이미지 포맷 (aspect mask 결정에 사용)
         * @param arrayLayers 이미지의 총 배열 레이어 수
         * @param newLayout 전환할 목표 이미지 레이아웃
         * @param newAccess 전환 후 접근 권한 마스크 (VkAccessFlags2)
         * @param newStage 전환 후 파이프라인 스테이지 (VkPipelineStageFlags2)
         * @param baseMipLevel 전환을 시작할 밉맵 레벨 (0부터 시작)
         * @param levelCount 전환할 밉맵 레벨 개수 (VK_REMAINING_MIP_LEVELS 사용 가능)
         * @param baseArrayLayer 전환을 시작할 배열 레이어 (0부터 시작)
         * @param layerCount 전환할 배열 레이어 개수 (VK_REMAINING_ARRAY_LAYERS 사용 가능)
         *
         * @details 함수 동작 과정:
         * 1. 입력 유효성 검사 (image가 VK_NULL_HANDLE인지 확인)
         * 2. VK_REMAINING_* 상수 처리로 실제 레벨/레이어 개수 계산
         * 3. 불필요한 전환 최적화 (동일한 레이아웃과 접근 권한, 전체 리소스 범위인 경우 스킵)
         * 4. VkImageMemoryBarrier2 구조체 설정:
         *    - 소스/목적지 파이프라인 스테이지 설정
         *    - 소스/목적지 접근 마스크 설정
         *    - 이전/새로운 이미지 레이아웃 설정
         *    - 서브리소스 범위 설정 (밉맵, 배열 레이어, aspect)
         * 5. VkDependencyInfo를 통한 vkCmdPipelineBarrier2 호출
         *
         * @note 최적화 특징:
         * - 동일한 레이아웃과 접근 권한이며 전체 리소스를 대상으로 하는 경우 전환을 생략
         * - VK_REMAINING_MIP_LEVELS와 VK_REMAINING_ARRAY_LAYERS 자동 계산 지원
         * - 깊이/스텐실 포맷에 대한 자동 aspect mask 결정
         *
         * @warning 주의사항:
         * - commandBuffer는 반드시 기록 상태(recording state)여야 합니다
         * - oldStage가 VK_PIPELINE_STAGE_2_NONE인 경우 VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT로 대체
         * - 큐 패밀리 전송은 지원하지 않음 (VK_QUEUE_FAMILY_IGNORED 사용)
         *
         * @example 사용 예시:
         *
         */
        void transitionImageLayout2(
            VkCommandBuffer commandBuffer,
            VkImage image,
            VkFormat format,
            VkImageLayout newLayout,
            VkAccessFlags2 newAccess,
            VkPipelineStageFlags2 newStage,
            cUint32_t baseMipLevel = 0,
            cUint32_t levelCount = VK_REMAINING_MIP_LEVELS,
            cUint32_t baseArrayLayer = 0,
            cUint32_t layerCount = VK_REMAINING_ARRAY_LAYERS
        );

    private:
        VkImageAspectFlags getAspectFlagsFromLayout(VkFormat format)
        {
            if (format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                format == VK_FORMAT_D24_UNORM_S8_UINT ||
                format == VK_FORMAT_D16_UNORM_S8_UINT ||
                format == VK_FORMAT_D16_UNORM) {
                return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            else if (format == VK_FORMAT_D32_SFLOAT ||
                format == VK_FORMAT_D16_UNORM ||
                format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
                format == VK_FORMAT_D16_UNORM_S8_UINT) {
                return VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            else {
                return VK_IMAGE_ASPECT_COLOR_BIT;
            }
        }

        bool isValidTransition(VkImageLayout oldLayout, VkImageLayout newLayout) const
        {
            if (oldLayout == newLayout || oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                return true;

            if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
                return newLayout == VK_IMAGE_LAYOUT_GENERAL ||
                    newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            }

            return true;
        }

        VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkAccessFlags2 currentAccess = VK_ACCESS_2_NONE;
        VkPipelineStageFlags2 currentStage = VK_PIPELINE_STAGE_2_NONE;

        uint32_t mipLevels{ 1 };
        uint32_t arrayLayers{ 1 };

    };

}

#endif // ! _VK_BARRIER_H_
