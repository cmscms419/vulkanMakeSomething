#ifndef INCLUDE_VULKANENGINE_H_
#define INCLUDE_VULKANENGINE_H_

#include "../_common.h"

#include "VKdevice.h"
#include "VKswapchain.h"

namespace vkengine {

    class VulkanEngine {
    public:
        VulkanEngine(std::string root_path);
        ~VulkanEngine();

        bool _isInitialized{ false };
        bool stop_rendering{ false };
        bool framebufferResized{ false };        // 프레임 버퍼 크기 조정 여부

        FrameData& getCurrnetFrameData();

        static VulkanEngine& Get();

        //initializes everything in the engine
        void init();

        //shuts down the engine
        void cleanup();

        //mainLoop
        bool mainLoop();

        // drawFrame
        void drawFrame();

        // create window
        void initWindow();

    protected:

        virtual bool init_vulkan();
        virtual bool init_swapchain();
        virtual bool init_commands();
        virtual bool init_sync_structures();
        virtual bool prepare();

        // 엔진 초기화
        virtual void createInstance();                             // 인스턴스 생성
        virtual void setupDebugCallback();                         // 디버그 메신저 생성
        virtual void createSurface();                              // 서피스 생성
        virtual void createDevice();                               // 디바이스(logical, pysical) 생성
        virtual void createDepthStencilResources();                // 깊이 스텐실 생성
        virtual void createRenderPass();                           // 렌더 패스 생성
        virtual void createPipelineCache();                        // 파이프라인 캐시 생성
        virtual void createFramebuffers();                         // 프레임 버퍼 생성

        void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex);                        // 커맨드 버퍼 레코드
        void cleanupSwapChain();                          // 스왑 체인 정리

        // 도구
        bool checkValidationLayerSupport();               // 검증 레이어 지원 확인
        std::vector<const char*> getRequiredExtensions(); // 필요한 확장 목록 가져오기

        GLFWwindow* VKwindow{ nullptr };  // GLFW 윈도우 핸들 -> GLFW 윈도우 핸들을 저장
        VkInstance VKinstance{};                              // Vulkan 인스턴스 -> Vulkan API를 사용하기 위한 인스턴스
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger{};  // 디버그 메신저 -> 디버깅을 위한 메신저
        VkSurfaceKHR VKsurface{ VK_NULL_HANDLE };  // 서피스 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스

        FrameData VKframeData[MAX_FRAMES_IN_FLIGHT];       // 프레임 데이터 -> 프레임 데이터 구조체
        VkRenderPass VKrenderPass{ VK_NULL_HANDLE };       // 렌더 패스 -> 렌더링 작업을 정의하는 데 사용
        std::vector<VkFramebuffer> VKswapChainFramebuffers; // 스왑 체인 프레임 버퍼 -> 스왑 체인 이미지를 렌더링할 때 사용 (프레임 버퍼는 이미지를 렌더링하는 데 사용)
        depthStencill VKdepthStencill{};  // 깊이 스텐실 -> 깊이 스텐실 이미지와 메모리

        VKSwapChain* VKswapChain{ VK_NULL_HANDLE };  // 스왑 체인 -> 스왑 체인 클래스
        VKDevice_* VKdevice{};  // 디바이스 -> GPU Logical,Physical struct Handle
        VkSampleCountFlagBits VKmsaaSamples = VK_SAMPLE_COUNT_1_BIT; // MSAA 샘플 -> MSAA 샘플 수
        VkPipelineCache VKpipelineCache{ VK_NULL_HANDLE }; // 파이프라인 캐시 -> 파이프라인 캐시를 생성

        std::string RootPath = "";                          // 루트 경로
        size_t currentFrame = 0;                            // 현재 프레임 인덱스
        bool state = false;                                         // 프로그램 상태 
    };

}

#endif // INCLUDE_VULKANENGINE_H_