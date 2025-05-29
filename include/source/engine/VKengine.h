#ifndef INCLUDE_VULKANENGINE_H_
#define INCLUDE_VULKANENGINE_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <map>

#include "VKdevice.h"
#include "VKbuffer.h"
#include "VKswapchain.h"
#include "VKinputEvent.h"
#include "Debug.h"
#include "helper.h"

namespace vkengine {

    namespace object {
        class Camera;
    }

    class VulkanEngine {
    public:
        VulkanEngine(std::string root_path);
        ~VulkanEngine();

        bool _isInitialized{ false };
        bool stop_rendering{ false };
        bool framebufferResized{ false };   // ������ ���� ũ�� ���� ����

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

        void createDescriptorPoolImGui();

        float getCalulateDeltaTime() {
            auto newTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            return deltaTime;
        }

        float getProgramRunTime() {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            return time;
        }

        bool isInitialized() const { return _isInitialized; }
        bool isStopRendering() const { return stop_rendering; }
        bool isFramebufferResized() const { return framebufferResized; }
        GLFWwindow* getWindow() const { return VKwindow; }
        VkInstance getInstance() const { return VKinstance; }
        VkDebugUtilsMessengerEXT getDebugUtilsMessenger() const { return VKdebugUtilsMessenger; }
        VkSurfaceKHR getSurface() const { return VKsurface; }
        const FrameData* getFrameData() { return VKframeData; }
        VkRenderPass getRenderPass() const { return *this->VKrenderPass.get(); }
        std::vector<VkFramebuffer> getSwapChainFramebuffers() const { return VKswapChainFramebuffers; }
        depthStencill getDepthStencill() const { return VKdepthStencill; }
        VKSwapChain* getSwapChain() const { return VKswapChain.get(); }
        VKDevice_* getDevice() const { return VKdevice.get(); }
        VkSampleCountFlagBits getMsaaSamples() const { return VKmsaaSamples; }
        VkPipelineCache getPipelineCache() const { return VKpipelineCache; }
        std::string getRootPath() const { return RootPath; }
        size_t getCurrentFrame() const { return currentFrame; }
        bool getState() const { return state; }
        VkDescriptorPool getDescriptorPool() const { return VKdescriptorPool; }
        VkDescriptorSetLayout getDescriptorSetLayout() const { return VKdescriptorSetLayout; }
        void setWindowWidth(int width) { windowWidth = width; }
        void setWindowHeight(int height) { windowHeight = height; }
        std::shared_ptr<vkengine::object::Camera> getCamera() { return camera; }
        
        void setKeyPressed(int key, bool value) { m_keyPressed[key] = value; }
        bool getKeyPressed(int key) { return m_keyPressed[key]; }
        void setMousePressed(int button, bool value) { m_mousePressed[button] = value; }
        bool getMousePressed(int button) { return m_mousePressed[button]; }
        
        void setMousePosition(float x, float y) { lastMouseX = x; lastMouseY = y; }
        float getLastMouseX() { return lastMouseX; }
        float getLastMouseY() { return lastMouseY; }
        void setCameraMoveCheck(bool check) { this->cameraMove = check; }
        bool getCameraMoveCheck() { return this->cameraMove; }
        void setCameraMoveStyle(bool style) { this->cameraMoveStyle = style; }
        bool getCameraMoveStyle() { return this->cameraMoveStyle; }

    protected:

        virtual bool initVulkan();
        virtual bool init_swapchain();
        virtual bool init_command_pool();
        virtual bool init_sync_structures();

        // ���� �ʱ�ȭ
        virtual bool createInstance();                             // �ν��Ͻ� ����
        virtual bool setupDebugCallback();                         // ����� �޽��� ����
        virtual bool createSurface();                              // ���ǽ� ����
        virtual bool createDevice();                               // ����̽�(logical, pysical) ����
        virtual bool createDepthStencilResources();                // ���� ���ٽ� ����
        virtual bool createRenderPass();                           // ���� �н� ����
        virtual bool createPipelineCache();                        // ���������� ĳ�� ����
        virtual bool createFramebuffers();                         // ������ ���� ����
        virtual bool recreateSwapChain();                          // ���� ü�� �����
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
        std::unique_ptr<VkRenderPass> VKrenderPass{ VK_NULL_HANDLE };        // ���� �н� -> ������ �۾��� �����ϴ� �� ���
        std::vector<VkFramebuffer> VKswapChainFramebuffers; // ���� ü�� ������ ���� -> ���� ü�� �̹����� �������� �� ��� (������ ���۴� �̹����� �������ϴ� �� ���)
        depthStencill VKdepthStencill{};  // ���� ���ٽ� -> ���� ���ٽ� �̹����� �޸�

        std::unique_ptr<VKSwapChain> VKswapChain;  // ���� ü�� -> ���� ü�� Ŭ����
        std::unique_ptr<VKDevice_> VKdevice{};  // ����̽� -> GPU Logical,Physical struct Handle
        VkPipelineCache VKpipelineCache{ VK_NULL_HANDLE }; // ���������� ĳ�� -> ���������� ĳ�ø� ����
        VkSampleCountFlagBits VKmsaaSamples = VK_SAMPLE_COUNT_1_BIT; // MSAA ���� -> MSAA ���� ��

        VkDescriptorPool VKdescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout VKdescriptorSetLayout{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> VKdescriptorSets = {};

        std::shared_ptr<vkengine::object::Camera> camera = nullptr;  // ī�޶� -> ī�޶� Ŭ����

        // �������� �����ϱ� ���� �������� �غ� �Ǿ����� Ȯ���ϴ� ����
        VkSubmitInfo VKsubmitInfo{};  // ���� ���� -> ������ ��� ���ۿ� ������� ����

        std::string RootPath = "";                          // ��Ʈ ���
        size_t currentFrame = 0;                            // ���� ������ �ε���
        bool state = false;                                 // ���α׷� ���� 
        int windowWidth = WIDTH;                            // ������ �ʺ�
        int windowHeight = HEIGHT;                          // ������ ����

        // ���� Ű���尡 ���ȴ��� ���¸� �����ϴ� �迭
        bool m_keyPressed[256] = {
            false,
        };

        // ���콺 Ŀ�� ��ġ
        float lastMouseX = 0;
        float lastMouseY = 0;

        bool m_mousePressed[3] = {
            false, // ���� ��ư
            false, // ������ ��ư
            false, // ��� ��ư
        };

        // Camera Move ����
        bool cameraMove = false;
        bool cameraMoveStyle = false; // ī�޶� �̵� ��Ÿ��

        // ���� �ð�
        std::chrono::high_resolution_clock::time_point currentTime;
        std::chrono::high_resolution_clock::time_point startTime;

        // �ִ� ������ ��
        uint16_t frames = MAX_FRAMES_IN_FLIGHT;
    };

}

#endif // INCLUDE_VULKANENGINE_H_