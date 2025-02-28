#ifndef INCLUDE_VULKANENGINE_H_
#define INCLUDE_VULKANENGINE_H_

#include "../_common.h"
#include "../struct.h"

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
        virtual void init();

        //shuts down the engine
        virtual void cleanup();

        //mainLoop
        virtual bool mainLoop();

        // create window
        virtual void initWindow();

        // prepare
        virtual bool prepare();

        // drawFrame
        virtual void drawFrame() = 0;

        // �������� �غ��ϴ� �Լ�
        void prepareFame(uint32_t* imageIndex);

        // present frame -> ȭ�鿡 �������� �̹����� ǥ��
        void presentFrame(uint32_t* imageIndex);

    public:
        bool isInitialized() const { return _isInitialized; }
        bool isStopRendering() const { return stop_rendering; }
        bool isFramebufferResized() const { return framebufferResized; }
        GLFWwindow* getWindow() const { return VKwindow; }
        VkInstance getInstance() const { return VKinstance; }
        VkDebugUtilsMessengerEXT getDebugUtilsMessenger() const { return VKdebugUtilsMessenger; }
        VkSurfaceKHR getSurface() const { return VKsurface; }
        const FrameData* getFrameData() { return VKframeData; }
        VkRenderPass getRenderPass() const { return VKrenderPass; }
        std::vector<VkFramebuffer> getSwapChainFramebuffers() const { return VKswapChainFramebuffers; }
        depthStencill getDepthStencill() const { return VKdepthStencill; }
        VKSwapChain* getSwapChain() const { return VKswapChain; }
        VKDevice_* getDevice() const { return VKdevice; }
        VkSampleCountFlagBits getMsaaSamples() const { return VKmsaaSamples; }
        VkPipelineCache getPipelineCache() const { return VKpipelineCache; }
        std::string getRootPath() const { return RootPath; }
        size_t getCurrentFrame() const { return currentFrame; }
        bool getState() const { return state; }
        VkDescriptorPool getDescriptorPool() const { return VKdescriptorPool; }
        VkDescriptorSetLayout getDescriptorSetLayout() const { return VKdescriptorSetLayout; }

    protected:

        virtual bool initVulkan();
        virtual bool init_swapchain();
        virtual bool init_command_pool();
        virtual bool init_sync_structures();

        // ���� �ʱ�ȭ
        virtual void createInstance();                             // �ν��Ͻ� ����
        virtual void setupDebugCallback();                         // ����� �޽��� ����
        virtual void createSurface();                              // ���ǽ� ����
        virtual void createDevice();                               // ����̽�(logical, pysical) ����
        virtual void createDepthStencilResources();                // ���� ���ٽ� ����
        virtual void createRenderPass();                           // ���� �н� ����
        virtual void createPipelineCache();                        // ���������� ĳ�� ����
        virtual void createFramebuffers();                         // ������ ���� ����
        virtual void recreateSwapChain();                          // ���� ü�� �����
        virtual bool createCommandBuffer();                        // Ŀ�ǵ� ���� ����

        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex);    // Ŀ�ǵ� ���� ���ڵ�

        // ����
        bool checkValidationLayerSupport();               // ���� ���̾� ���� Ȯ��
        std::vector<const char*> getRequiredExtensions(); // �ʿ��� Ȯ�� ��� ��������

        GLFWwindow* VKwindow{ nullptr };  // GLFW ������ �ڵ� -> GLFW ������ �ڵ��� ����
        VkInstance VKinstance{};                              // Vulkan �ν��Ͻ� -> Vulkan API�� ����ϱ� ���� �ν��Ͻ�
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger{};  // ����� �޽��� -> ������� ���� �޽���
        VkSurfaceKHR VKsurface{ VK_NULL_HANDLE };  // ���ǽ� -> ������ �ý��۰� Vulkan�� �����ϴ� �������̽�

        FrameData VKframeData[MAX_FRAMES_IN_FLIGHT];        // ������ ������ -> ������ ������ ����ü
        VkRenderPass VKrenderPass{ VK_NULL_HANDLE };        // ���� �н� -> ������ �۾��� �����ϴ� �� ���
        std::vector<VkFramebuffer> VKswapChainFramebuffers; // ���� ü�� ������ ���� -> ���� ü�� �̹����� �������� �� ��� (������ ���۴� �̹����� �������ϴ� �� ���)
        depthStencill VKdepthStencill{};  // ���� ���ٽ� -> ���� ���ٽ� �̹����� �޸�

        VKSwapChain* VKswapChain{ VK_NULL_HANDLE };  // ���� ü�� -> ���� ü�� Ŭ����
        VKDevice_* VKdevice{};  // ����̽� -> GPU Logical,Physical struct Handle
        VkSampleCountFlagBits VKmsaaSamples = VK_SAMPLE_COUNT_1_BIT; // MSAA ���� -> MSAA ���� ��
        VkPipelineCache VKpipelineCache{ VK_NULL_HANDLE }; // ���������� ĳ�� -> ���������� ĳ�ø� ����

        VkDescriptorPool VKdescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout VKdescriptorSetLayout{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> VKdescriptorSets = {};

        // �������� �����ϱ� ���� �������� �غ� �Ǿ����� Ȯ���ϴ� ����
        VkSubmitInfo VKsubmitInfo{};  // ���� ���� -> ������ ��� ���ۿ� ������� ����

        std::string RootPath = "";                          // ��Ʈ ���
        size_t currentFrame = 0;                            // ���� ������ �ε���
        bool state = false;                                         // ���α׷� ���� 
    };

}

#endif // INCLUDE_VULKANENGINE_H_