#ifndef INCLUDE_VULKANENGINE_H_
#define INCLUDE_VULKANENGINE_H_

#include "../_common.h"
#include "VKdevice.h"

namespace vkengine {
    struct VKDevice_;                                  // 디바이스 -> GPU Logical,Physical struct Handle

    class VulkanEngine {
    public:
        VulkanEngine() = default;
        ~VulkanEngine() = default;

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
        void init_sync_structures();

        // 엔진 초기화
        void createInstance();                             // 인스턴스 생성
        void setupDebugCallback();                         // 디버그 메신저 생성
        void createSurface();                              // 서피스 생성
        void createDevice();                               // 디바이스(logical, pysical) 생성

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
        VkSwapchainKHR VKswapChain;                         // 스왑 체인 -> 이미지를 프레임 버퍼로 전송하는 데 사용
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger;     // 디버그 메신저 -> 디버깅을 위한 메신저
        VKDevice_* VKdevice;                                // 디바이스 -> GPU Logical,Physical struct Handle
        VkSurfaceKHR VKsurface;                             // 서피스 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스

        std::vector<VkImage> VKswapChainImages;             // 스왑 체인 이미지 -> 스왑 체인에 사용되는 이미지 Handle 배열
        std::vector<VkImageView> VKswapChainImageViews;     // 스왑 체인 이미지 뷰 -> 스왑 체인 이미지를 뷰로 변환 (이미지 뷰는 이미지를 읽고 쓰는 데 사용됨)

        std::string RootPath = "";                          // 루트 경로
        size_t currentFrame = 0;                            // 현재 프레임 인덱스
        bool state;                                         // 프로그램 상태 
    };

}

#endif // INCLUDE_VULKANENGINE_H_