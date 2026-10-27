#ifndef INCLUDE_VK_CONTEXT_H_
#define INCLUDE_VK_CONTEXT_H_

#include "common.h"
#include "vkdevice.h"

#include "CommandBufferHandle.h"
#include "DeviceManager2.h"
#include "DescriptorManager2.h"

namespace vkengine {
    class Context
    {
    public:

        // Context 클래스에 추가
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        Context(const std::vector<const char*>& requiredInstanceExtensions, cBool useSwapchain);
        ~Context();

        void cleanup();
        virtual cBool createInstance(std::vector<const char*> requiredInstanceExtensions);  // 인스턴스 생성
        virtual cBool createPhysicalDevice();                                                // 디바이스(logical, pysical)
        virtual cBool createLogicalDevice(cBool useSwapchain);                              // 물리 디바이스 선택
        virtual cBool createCommandPools();                                                 // 커맨드 풀 생성 (graphic, compute, transfer)
        virtual cBool createQueues();                                                       // 큐 생성 (graphic, compute, transfer)
        virtual cBool createPipelineCache();                                                // 파이프라인 캐시 생성
        virtual cBool createDepthStencilFormat();                                           // 깊이 스텐실 포맷 생성
        virtual cBool createDebugMessenger();

        // 도구
        cBool checkValidationLayerSupport(const cChar* str); // 검증 레이어 지원 확인
        CommandBufferHandle createGraphicsCommandBufferHandle(VkCommandBufferLevel level, cBool begin); // 커맨드 버퍼 생성
        std::vector<CommandBufferHandle> createGraphicsCommandBufferHandles(cUint32_t count); // 여러개의 graphics 커맨드 버퍼 생성
        CommandBufferHandle createTransferCommandBufferHandle(VkCommandBufferLevel level, cBool begin);
        std::vector<CommandBufferHandle> createTransferCommandBufferHandles(cUint32_t count); // 여러개의 graphics 커맨드 버퍼 생성

        
        cUint32_t getMemoryTypeIndex(cUint32_t typeBits, VkMemoryPropertyFlags properties);

        VkQueue getGraphicsQueue() { return this->VKdevice.graphicsVKQueue; }
        VkQueue getComputeQueue() { return this->VKdevice.computeVKQueue; }
        VkQueue getTransferQueue() { return this->VKdevice.transferVKQueue; }

        DeviceManager2* getDevice() { return &VKdevice; }
        DescriptorManager2* getDescriptorManager() { return &descriptorManager2; }
        VkInstance getInstance() { return VKinstance; }
        VkPipelineCache getPipelineCache() { return VKpipelineCache; }
        DepthStencil* getDepthStencil() { return &VKdepthStencil; }
        cBool isDepthStencilUsed() { return useDepthStencil; }

        void waitIdle();
        void waitGraphicsQueueIdle();

    private:
        VkInstance VKinstance;                              // Vulkan 인스턴스 -> Vulkan API를 사용하기 위한 인스턴스
        DeviceManager2 VKdevice;                            // 디바이스 -> GPU Logical,Physical struct Handle
        VkPipelineCache VKpipelineCache;    // 파이프라인 캐시 -> 파이프라인 캐시를 생성
        DepthStencil VKdepthStencil;                      // 깊이 스텐실 -> 깊이 스텐실 이미지와 메모리
        DescriptorManager2 descriptorManager2;               // 디스크립터 매니저 -> 디스크립터 세트 레이아웃과 디스크립터 풀 관리
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger;  // 디버그 메신저 -> 디버깅을 위한 메신저
        cBool useDepthStencil;
    };
}

#endif // !INCLUDE_VK_CONTEXT_H_
