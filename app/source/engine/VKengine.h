#ifndef INCLUDE_VULKANENGINE_H_
#define INCLUDE_VULKANENGINE_H_

#include "../_common.h"
#include "VKdevice.h"

namespace vkengine {
    struct VKDevice_;                                  // ����̽� -> GPU Logical,Physical struct Handle

    class VulkanEngine {
    public:
        VulkanEngine() = default;
        ~VulkanEngine() = default;

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
        void init_sync_structures();

        // ���� �ʱ�ȭ
        void createInstance();                             // �ν��Ͻ� ����
        void setupDebugCallback();                         // ����� �޽��� ����
        void createSurface();                              // ���ǽ� ����
        void createDevice();                               // ����̽�(logical, pysical) ����

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
        VkSwapchainKHR VKswapChain;                         // ���� ü�� -> �̹����� ������ ���۷� �����ϴ� �� ���
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger;     // ����� �޽��� -> ������� ���� �޽���
        VKDevice_* VKdevice;                                // ����̽� -> GPU Logical,Physical struct Handle
        VkSurfaceKHR VKsurface;                             // ���ǽ� -> ������ �ý��۰� Vulkan�� �����ϴ� �������̽�

        std::vector<VkImage> VKswapChainImages;             // ���� ü�� �̹��� -> ���� ü�ο� ���Ǵ� �̹��� Handle �迭
        std::vector<VkImageView> VKswapChainImageViews;     // ���� ü�� �̹��� �� -> ���� ü�� �̹����� ��� ��ȯ (�̹��� ��� �̹����� �а� ���� �� ����)

        std::string RootPath = "";                          // ��Ʈ ���
        size_t currentFrame = 0;                            // ���� ������ �ε���
        bool state;                                         // ���α׷� ���� 
    };

}

#endif // INCLUDE_VULKANENGINE_H_