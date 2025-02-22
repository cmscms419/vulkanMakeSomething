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
        bool framebufferResized{ false };        // ������ ���� ũ�� ���� ����

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

        // ���� �ʱ�ȭ
        virtual void createInstance();                             // �ν��Ͻ� ����
        virtual void setupDebugCallback();                         // ����� �޽��� ����
        virtual void createSurface();                              // ���ǽ� ����
        virtual void createDevice();                               // ����̽�(logical, pysical) ����
        virtual void createDepthStencilResources();                // ���� ���ٽ� ����
        virtual void createRenderPass();                           // ���� �н� ����
        virtual void createPipelineCache();                        // ���������� ĳ�� ����
        virtual void createFramebuffers();                         // ������ ���� ����

        void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex);                        // Ŀ�ǵ� ���� ���ڵ�
        void cleanupSwapChain();                          // ���� ü�� ����

        // ����
        bool checkValidationLayerSupport();               // ���� ���̾� ���� Ȯ��
        std::vector<const char*> getRequiredExtensions(); // �ʿ��� Ȯ�� ��� ��������

        GLFWwindow* VKwindow{ nullptr };  // GLFW ������ �ڵ� -> GLFW ������ �ڵ��� ����
        VkInstance VKinstance{};                              // Vulkan �ν��Ͻ� -> Vulkan API�� ����ϱ� ���� �ν��Ͻ�
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger{};  // ����� �޽��� -> ������� ���� �޽���
        VkSurfaceKHR VKsurface{ VK_NULL_HANDLE };  // ���ǽ� -> ������ �ý��۰� Vulkan�� �����ϴ� �������̽�

        FrameData VKframeData[MAX_FRAMES_IN_FLIGHT];       // ������ ������ -> ������ ������ ����ü
        VkRenderPass VKrenderPass{ VK_NULL_HANDLE };       // ���� �н� -> ������ �۾��� �����ϴ� �� ���
        std::vector<VkFramebuffer> VKswapChainFramebuffers; // ���� ü�� ������ ���� -> ���� ü�� �̹����� �������� �� ��� (������ ���۴� �̹����� �������ϴ� �� ���)
        depthStencill VKdepthStencill{};  // ���� ���ٽ� -> ���� ���ٽ� �̹����� �޸�

        VKSwapChain* VKswapChain{ VK_NULL_HANDLE };  // ���� ü�� -> ���� ü�� Ŭ����
        VKDevice_* VKdevice{};  // ����̽� -> GPU Logical,Physical struct Handle
        VkSampleCountFlagBits VKmsaaSamples = VK_SAMPLE_COUNT_1_BIT; // MSAA ���� -> MSAA ���� ��
        VkPipelineCache VKpipelineCache{ VK_NULL_HANDLE }; // ���������� ĳ�� -> ���������� ĳ�ø� ����

        std::string RootPath = "";                          // ��Ʈ ���
        size_t currentFrame = 0;                            // ���� ������ �ε���
        bool state = false;                                         // ���α׷� ���� 
    };

}

#endif // INCLUDE_VULKANENGINE_H_