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

#define MOVESPEED 1.2f

namespace vkengine {

    namespace object {
        class Camera;
    }

    class VulkanEngine {
    public:
        VulkanEngine(std::string root_path);
        ~VulkanEngine();

        cBool _isInitialized{ false };
        cBool stop_rendering{ false };
        cBool framebufferResized{ false };   // 프레임 버퍼 크기 조정 여부

        FrameData& getCurrnetFrameData();

        static VulkanEngine& Get();

        //initializes everything in the engine
        virtual void init();

        //shuts down the engine
        virtual void cleanup();

        //mainLoop
        virtual cBool mainLoop();

        // create window
        virtual void initWindow();

        // prepare
        virtual cBool prepare();

        // drawFrame
        virtual void drawFrame() = 0;

        // 프레임을 준비하는 함수
        void prepareFame(uint32_t* imageIndex);

        // present frame -> 화면에 렌더링된 이미지를 표시
        void presentFrame(uint32_t* imageIndex);

        void createDescriptorPoolImGui();

        cFloat getCalulateDeltaTime() {
            auto newTime = std::chrono::high_resolution_clock::now();
            cFloat deltaTime = std::chrono::duration<cFloat, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            return deltaTime;
        }

        cFloat getProgramRunTime() {
            auto currentTime = std::chrono::high_resolution_clock::now();
            cFloat time = std::chrono::duration<cFloat, std::chrono::seconds::period>(currentTime - startTime).count();

            return time;
        }

        cBool isInitialized() const { return _isInitialized; }
        cBool isStopRendering() const { return stop_rendering; }
        cBool isFramebufferResized() const { return framebufferResized; }
        GLFWwindow* getWindow() const { return VKwindow; }
        VkInstance getInstance() const { return VKinstance; }
        VkDebugUtilsMessengerEXT getDebugUtilsMessenger() const { return VKdebugUtilsMessenger; }
        VkSurfaceKHR getSurface() const { return VKsurface; }
        const FrameData* getFrameData() { return VKframeData; }
        VkRenderPass getRenderPass() { return *this->VKrenderPass.get(); }
        std::vector<VkFramebuffer> getSwapChainFramebuffers() const { return VKswapChainFramebuffers; }
        depthStencill getDepthStencill() const { return VKdepthStencill; }
        VKSwapChain* getSwapChain() const { return VKswapChain.get(); }
        VKdeviceHandler* getDevice() const { return VKdevice.get(); }
        VkSampleCountFlagBits getMsaaSamples() const { return VKmsaaSamples; }
        VkPipelineCache getPipelineCache() const { return VKpipelineCache; }
        std::string getRootPath() const { return RootPath; }
        size_t getCurrentFrame() const { return currentFrame; }
        cBool getState() const { return state; }
        VkDescriptorPool getDescriptorPool() const { return VKdescriptorPool; }
        VkDescriptorSetLayout getDescriptorSetLayout() const { return VKdescriptorSetLayout; }
        void setWindowWidth(int width) { windowWidth = width; }
        void setWindowHeight(int height) { windowHeight = height; }
        std::shared_ptr<vkengine::object::Camera> getCamera() { return camera; }
        
        void setKeyPressed(int key, cBool value) { m_keyPressed[key] = value; }
        cBool getKeyPressed(int key) { return m_keyPressed[key]; }
        void setMousePressed(int button, cBool value) { m_mousePressed[button] = value; }
        cBool getMousePressed(int button) { return m_mousePressed[button]; }
        
        void setMousePosition(cFloat x, cFloat y) { lastMouseX = x; lastMouseY = y; }
        cFloat getLastMouseX() { return lastMouseX; }
        cFloat getLastMouseY() { return lastMouseY; }
        void setCameraMoveCheck(cBool check) { this->cameraMove = check; }
        cBool getCameraMoveCheck() { return this->cameraMove; }
        void setCameraMoveStyle(cBool style) { this->cameraMoveStyle = style; }
        cBool getCameraMoveStyle() { return this->cameraMoveStyle; }

    protected:

        virtual cBool initVulkan();
        virtual cBool init_swapchain();
        virtual cBool init_command_pool();
        virtual cBool init_sync_structures();

        // 엔진 초기화
        virtual cBool createInstance();                             // 인스턴스 생성
        virtual cBool setupDebugCallback();                         // 디버그 메신저 생성
        virtual cBool createSurface();                              // 서피스 생성
        virtual cBool createDevice();                               // 디바이스(logical, pysical) 생성
        virtual cBool createDepthStencilResources();                // 깊이 스텐실 생성
        virtual cBool createRenderPass();                           // 렌더 패스 생성
        virtual cBool createPipelineCache();                        // 파이프라인 캐시 생성
        virtual cBool createFramebuffers();                         // 프레임 버퍼 생성
        virtual cBool recreateSwapChain();                          // 스왑 체인 재생성
        virtual cBool createCommandBuffer();                        // 커맨드 버퍼 생성

        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex);    // 커맨드 버퍼 레코드

        // 도구
        cBool checkValidationLayerSupport();               // 검증 레이어 지원 확인
        std::vector<const cChar*> getRequiredExtensions(); // 필요한 확장 목록 가져오기

        GLFWwindow* VKwindow{ nullptr };  // GLFW 윈도우 핸들 -> GLFW 윈도우 핸들을 저장
        VkInstance VKinstance{};                              // Vulkan 인스턴스 -> Vulkan API를 사용하기 위한 인스턴스
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger{};  // 디버그 메신저 -> 디버깅을 위한 메신저
        VkSurfaceKHR VKsurface{ VK_NULL_HANDLE };  // 서피스 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스

        FrameData VKframeData[MAX_FRAMES_IN_FLIGHT];        // 프레임 데이터 -> 프레임 데이터 구조체
        std::unique_ptr<VkRenderPass> VKrenderPass{ VK_NULL_HANDLE };        // 렌더 패스 -> 렌더링 작업을 정의하는 데 사용
        std::vector<VkFramebuffer> VKswapChainFramebuffers; // 스왑 체인 프레임 버퍼 -> 스왑 체인 이미지를 렌더링할 때 사용 (프레임 버퍼는 이미지를 렌더링하는 데 사용)
        depthStencill VKdepthStencill{};  // 깊이 스텐실 -> 깊이 스텐실 이미지와 메모리

        std::unique_ptr<VKSwapChain> VKswapChain;  // 스왑 체인 -> 스왑 체인 클래스
        std::unique_ptr<VKdeviceHandler> VKdevice{};  // 디바이스 -> GPU Logical,Physical struct Handle
        VkPipelineCache VKpipelineCache{ VK_NULL_HANDLE }; // 파이프라인 캐시 -> 파이프라인 캐시를 생성
        VkSampleCountFlagBits VKmsaaSamples = VK_SAMPLE_COUNT_1_BIT; // MSAA 샘플 -> MSAA 샘플 수

        VkDescriptorPool VKdescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout VKdescriptorSetLayout{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> VKdescriptorSets = {};

        std::shared_ptr<vkengine::object::Camera> camera = nullptr;  // 카메라 -> 카메라 클래스

        // 렌더링을 시작하기 전에 렌더링할 준비가 되었는지 확인하는 변수
        VkSubmitInfo VKsubmitInfo{};  // 제출 정보 -> 제출할 명령 버퍼와 세마포어를 저장

        std::string RootPath = "";                          // 루트 경로
        size_t currentFrame = 0;                            // 현재 프레임 인덱스
        cBool state = false;                                 // 프로그램 상태 
        int windowWidth = WIDTH;                            // 윈도우 너비
        int windowHeight = HEIGHT;                          // 윈도우 높이

        // 현재 키보드가 눌렸는지 상태를 저장하는 배열
        cBool m_keyPressed[256] = {
            false,
        };

        // 마우스 커서 위치
        cFloat lastMouseX = 0;
        cFloat lastMouseY = 0;

        cBool m_mousePressed[3] = {
            false, // 왼쪽 버튼
            false, // 오른쪽 버튼
            false, // 가운데 버튼
        };

        // Camera Move 여부
        cBool cameraMove = false;
        cBool cameraMoveStyle = false; // 카메라 이동 스타일

        // 현재 시간
        std::chrono::high_resolution_clock::time_point currentTime;
        std::chrono::high_resolution_clock::time_point startTime;

        // 최대 프레임 수
        uint16_t frames = MAX_FRAMES_IN_FLIGHT;
    };

}

#endif // INCLUDE_VULKANENGINE_H_