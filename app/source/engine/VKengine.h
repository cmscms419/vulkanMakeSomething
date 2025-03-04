#ifndef INCLUDE_VULKANENGINE_H_
#define INCLUDE_VULKANENGINE_H_

#include "../_common.h"
#include "../struct.h"

#include "VKdevice.h"
#include "VKswapchain.h"

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
        bool framebufferResized{ false };        // 프레임 버퍼 크기 조정 여부

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

        // 프레임을 준비하는 함수
        void prepareFame(uint32_t* imageIndex);

        // present frame -> 화면에 렌더링된 이미지를 표시
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

    protected:

        virtual bool initVulkan();
        virtual bool init_swapchain();
        virtual bool init_command_pool();
        virtual bool init_sync_structures();

        // 엔진 초기화
        virtual void createInstance();                             // 인스턴스 생성
        virtual void setupDebugCallback();                         // 디버그 메신저 생성
        virtual void createSurface();                              // 서피스 생성
        virtual void createDevice();                               // 디바이스(logical, pysical) 생성
        virtual void createDepthStencilResources();                // 깊이 스텐실 생성
        virtual void createRenderPass();                           // 렌더 패스 생성
        virtual void createPipelineCache();                        // 파이프라인 캐시 생성
        virtual void createFramebuffers();                         // 프레임 버퍼 생성
        virtual void recreateSwapChain();                          // 스왑 체인 재생성
        virtual bool createCommandBuffer();                        // 커맨드 버퍼 생성

        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex);    // 커맨드 버퍼 레코드

        // 도구
        bool checkValidationLayerSupport();               // 검증 레이어 지원 확인
        std::vector<const char*> getRequiredExtensions(); // 필요한 확장 목록 가져오기

        GLFWwindow* VKwindow{ nullptr };  // GLFW 윈도우 핸들 -> GLFW 윈도우 핸들을 저장
        VkInstance VKinstance{};                              // Vulkan 인스턴스 -> Vulkan API를 사용하기 위한 인스턴스
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger{};  // 디버그 메신저 -> 디버깅을 위한 메신저
        VkSurfaceKHR VKsurface{ VK_NULL_HANDLE };  // 서피스 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스

        FrameData VKframeData[MAX_FRAMES_IN_FLIGHT];        // 프레임 데이터 -> 프레임 데이터 구조체
        std::unique_ptr<VkRenderPass> VKrenderPass{ VK_NULL_HANDLE };        // 렌더 패스 -> 렌더링 작업을 정의하는 데 사용
        std::vector<VkFramebuffer> VKswapChainFramebuffers; // 스왑 체인 프레임 버퍼 -> 스왑 체인 이미지를 렌더링할 때 사용 (프레임 버퍼는 이미지를 렌더링하는 데 사용)
        depthStencill VKdepthStencill{};  // 깊이 스텐실 -> 깊이 스텐실 이미지와 메모리

        std::unique_ptr<VKSwapChain> VKswapChain;  // 스왑 체인 -> 스왑 체인 클래스
        std::unique_ptr<VKDevice_> VKdevice{};  // 디바이스 -> GPU Logical,Physical struct Handle
        VkSampleCountFlagBits VKmsaaSamples = VK_SAMPLE_COUNT_1_BIT; // MSAA 샘플 -> MSAA 샘플 수
        VkPipelineCache VKpipelineCache{ VK_NULL_HANDLE }; // 파이프라인 캐시 -> 파이프라인 캐시를 생성

        VkDescriptorPool VKdescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout VKdescriptorSetLayout{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> VKdescriptorSets = {};

        std::shared_ptr<vkengine::object::Camera> camera = nullptr;  // 카메라 -> 카메라 클래스

        // 렌더링을 시작하기 전에 렌더링할 준비가 되었는지 확인하는 변수
        VkSubmitInfo VKsubmitInfo{};  // 제출 정보 -> 제출할 명령 버퍼와 세마포어를 저장

        std::string RootPath = "";                          // 루트 경로
        size_t currentFrame = 0;                            // 현재 프레임 인덱스
        bool state = false;                                         // 프로그램 상태 
        int windowWidth = WIDTH;                                  // 윈도우 너비
        int windowHeight = HEIGHT;                                // 윈도우 높이

        // 현재 키보드가 눌렸는지 상태를 저장하는 배열
        bool m_keyPressed[256] = {
            false,
        };

        // 마우스 커서 위치
        int currnetX = 0;
        int currnetY = 0;
    };

}

#endif // INCLUDE_VULKANENGINE_H_