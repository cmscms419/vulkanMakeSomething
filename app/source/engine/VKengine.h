#ifndef INCLUDE_VULKANENGINE_H_
#define INCLUDE_VULKANENGINE_H_

#include "../_common.h"
#include "VKdevice.h"

namespace vkcms {
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

        void createInstance();
        void setupDebugCallback();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSurface();
        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffers();
        void createSyncObjects();
        void cleanupSwapChain();
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


        GLFWwindow* VKwindow;                               // GLFW 윈도우 핸들 -> GLFW 윈도우 핸들을 저장
        VkInstance VKinstance;                              // Vulkan 인스턴스 -> Vulkan API를 사용하기 위한 인스턴스
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger;     // 디버그 메신저 -> 디버깅을 위한 메신저
        VKDevice_ VKdevice;                                  // 논리 디바이스 -> GPU Logical Handle

        std::string RootPath = "";                          // 루트 경로
        size_t currentFrame = 0;                            // 현재 프레임 인덱스
        bool state;                                         // 프로그램 상태 
    };

}

#endif // INCLUDE_VULKANENGINE_H_