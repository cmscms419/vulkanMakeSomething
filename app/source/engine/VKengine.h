#ifndef INCLUDE_VULKANENGINE_H_
#define INCLUDE_VULKANENGINE_H_

#include "../_common.h"

#include "VKdevice.h"
#include "VKswapchain.h"

namespace vkengine {

    class VulkanEngine {
    public:
        VulkanEngine() = default;
        ~VulkanEngine();

        bool _isInitialized{ false };
        bool stop_rendering{ false };
        bool framebufferResized{ false };        // 프레임 버퍼 크기 조정 여부

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

    private:

        void init_vulkan();
        void init_swapchain();
        void init_commands();
        bool init_sync_structures();

        // 엔진 초기화
        void createInstance();                             // 인스턴스 생성
        void setupDebugCallback();                         // 디버그 메신저 생성
        void createSurface();                              // 서피스 생성
        void createDevice();                               // 디바이스(logical, pysical) 생성

        void createDepthStencilResources();                          // 깊이 스텐실 생성

        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffers();
        void createSyncObjects();
        void recreateSwapChain();
        void createVertexBuffer();
        void createIndexBuffer();
        void createDescriptorSetLayout();
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        void createTextureImage();
        void createTextureImageView();
        void createTextureSampler();
        void createDepthResources();
        void loadModel();

        void cleanupSwapChain();                          // 스왑 체인 정리
        
        // 도구
        bool checkValidationLayerSupport();               // 검증 레이어 지원 확인
        std::vector<const char*> getRequiredExtensions(); // 필요한 확장 목록 가져오기

        GLFWwindow* VKwindow;                               // GLFW 윈도우 핸들 -> GLFW 윈도우 핸들을 저장
        VkInstance VKinstance;                              // Vulkan 인스턴스 -> Vulkan API를 사용하기 위한 인스턴스
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger;     // 디버그 메신저 -> 디버깅을 위한 메신저
        VkSurfaceKHR VKsurface;                             // 서피스 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스

        FrameData VKframeData[MAX_FRAMES_IN_FLIGHT];                              // 프레임 데이터 -> 프레임 데이터 구조체
        
        depthStencill VKdepthStencill;                      // 깊이 스텐실 -> 깊이 스텐실 이미지와 메모리

        VKSwapChain* VKswapChain;                            // 스왑 체인 -> 스왑 체인 클래스
        VKDevice_* VKdevice;                                // 디바이스 -> GPU Logical,Physical struct Handle
        VkSampleCountFlagBits VKmsaaSamples = VK_SAMPLE_COUNT_1_BIT; // MSAA 샘플 -> MSAA 샘플 수

        std::string RootPath = "";                          // 루트 경로
        size_t currentFrame = 0;                            // 현재 프레임 인덱스
        bool state;                                         // 프로그램 상태 
    };

}

#endif // INCLUDE_VULKANENGINE_H_