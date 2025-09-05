#ifndef INCLUDE_VK_CONTEXT_H_
#define INCLUDE_VK_CONTEXT_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKdevice2.h"
#include "VKDescriptorManager2.h"
#include "Debug.h"

namespace vkengine {
    class VkContext
    {
    public:
        VkContext(const std::vector<const char*>& requiredInstanceExtensions, cBool useSwapchain);
        ~VkContext();

        void cleanup();
        virtual cBool createInstance(std::vector<const char*> requiredInstanceExtensions);  // 인스턴스 생성
        virtual cBool createPysicalDevice();                                                // 디바이스(logical, pysical)
        virtual cBool createLogicalDevice(cBool useSwapchain);                              // 물리 디바이스 선택
        virtual cBool createCommandPools();                                                 // 커맨드 풀 생성 (grapic, compute, transfer)
        virtual cBool createQueues();                                                       // 큐 생성 (grapic, compute, transfer)
        virtual cBool createPipelineCache();                                                // 파이프라인 캐시 생성
        virtual cBool createDepthStencilFormat();                                           // 깊이 스텐실 포맷 생성

        // 도구
        cBool checkValidationLayerSupport(const cChar* str); // 검증 레이어 지원 확인

    private:
        VkInstance VKinstance{};                              // Vulkan 인스턴스 -> Vulkan API를 사용하기 위한 인스턴스
        VKdeviceHandler2 VKdevice;                            // 디바이스 -> GPU Logical,Physical struct Handle
        VkPipelineCache VKpipelineCache{ VK_NULL_HANDLE };    // 파이프라인 캐시 -> 파이프라인 캐시를 생성
        depthStencill VKdepthStencill{};                      // 깊이 스텐실 -> 깊이 스텐실 이미지와 메모리
        DescriptorManager2 descriptorManager2;               // 디스크립터 매니저 -> 디스크립터 세트 레이아웃과 디스크립터 풀 관리
    };
}

#endif // !INCLUDE_VK_CONTEXT_H_
