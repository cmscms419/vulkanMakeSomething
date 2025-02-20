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
        bool framebufferResized{ false };        // ������ ���� ũ�� ���� ����

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

        // ���� �ʱ�ȭ
        void createInstance();                             // �ν��Ͻ� ����
        void setupDebugCallback();                         // ����� �޽��� ����
        void createSurface();                              // ���ǽ� ����
        void createDevice();                               // ����̽�(logical, pysical) ����

        void createDepthStencilResources();                          // ���� ���ٽ� ����

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

        void cleanupSwapChain();                          // ���� ü�� ����
        
        // ����
        bool checkValidationLayerSupport();               // ���� ���̾� ���� Ȯ��
        std::vector<const char*> getRequiredExtensions(); // �ʿ��� Ȯ�� ��� ��������

        GLFWwindow* VKwindow;                               // GLFW ������ �ڵ� -> GLFW ������ �ڵ��� ����
        VkInstance VKinstance;                              // Vulkan �ν��Ͻ� -> Vulkan API�� ����ϱ� ���� �ν��Ͻ�
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger;     // ����� �޽��� -> ������� ���� �޽���
        VkSurfaceKHR VKsurface;                             // ���ǽ� -> ������ �ý��۰� Vulkan�� �����ϴ� �������̽�

        FrameData VKframeData[MAX_FRAMES_IN_FLIGHT];                              // ������ ������ -> ������ ������ ����ü
        
        depthStencill VKdepthStencill;                      // ���� ���ٽ� -> ���� ���ٽ� �̹����� �޸�

        VKSwapChain* VKswapChain;                            // ���� ü�� -> ���� ü�� Ŭ����
        VKDevice_* VKdevice;                                // ����̽� -> GPU Logical,Physical struct Handle
        VkSampleCountFlagBits VKmsaaSamples = VK_SAMPLE_COUNT_1_BIT; // MSAA ���� -> MSAA ���� ��

        std::string RootPath = "";                          // ��Ʈ ���
        size_t currentFrame = 0;                            // ���� ������ �ε���
        bool state;                                         // ���α׷� ���� 
    };

}

#endif // INCLUDE_VULKANENGINE_H_