#ifndef INCLUDE_SOURCE_APPLICATION_H
#define INCLUDE_SOURCE_APPLICATION_H

#include "cms491_common.h"

constexpr int MAX_FRAMES = 4;
constexpr auto shaderRootPath = "shader/";

namespace vkutil {

    class Application {
    public:
        Application();
        ~Application();

        virtual void init();
        virtual bool run();
        virtual void update();
        virtual void setup();
        virtual void cleanup();

    private:

        // GLFW 윈도우 생성
        // 상속
        virtual void initWindow();
        virtual void initVulkan();
        virtual void mainLoop();
        
        // Vulkan 인스턴스 생성
        // 기본
        void createInstance();
        void setupDebugCallback();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSurface();
        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createGraphicsPipeline();

        // 도구

        // 주어진 물리 장치에서 큐 패밀리 속성을 찾는 함수
        // PROB : 큐 패밀리가 여러개인 경우에 필요한 처리가 있는 패밀리를 먼저 찾을 경우, 그 패밀리의 인덱스만 반환함
        // TODO ; 큐 패밀리가 여러개인 경우에 대한 처리가 필요함
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        
        bool isDeviceSuitable(VkPhysicalDevice device);
        
        // 검증 레이어 지원 여부를 확인하는 함수
        // 확장 기능을 열거하고 필요한 모든 확장 기능이 포함되어 있는지 확인
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        // 스왑 체인 지원 정보를 가져오는 함수
        // 스왑 체인 지원 정보를 저장할 구조체를 초기화
        // 물리 장치에서 서피스의 기능을 가져옴
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        
        // 서피스 포맷을 선택하는 함수
        // SRGB 색상 공간을 지원하는 32비트 BGR 색상 구조를 선택
        // 이러한 형식이 지원되지 않으면 첫 번째 형식을 반환
        // 이러한 형식이 없으면 예외를 발생시킵니다.
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        // 프레젠테이션 모드를 선택하는 함수
        // VK_PRESENT_MODE_MAILBOX_KHR 프레젠테이션 모드를 지원하는지 확인
        // 지원되는 경우 VK_PRESENT_MODE_MAILBOX_KHR를 반환
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        
        // 스왑 체인 이미지의 해상도를 선택합니다.
        // 현재 해상도가 유효하면 반환하고, 그렇지 않으면 GLFW 크기를 기준으로 해상도를 설정합니다.
        // 해상도는 최소 및 최대 이미지 해상도 사이에서 클램프됩니다.
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
        // 코드를 읽어서 셰이더 모듈을 생성하는 함수
        VkShaderModule createShaderModule(const std::vector<char>& code);
        
        GLFWwindow* VKwindow;                               // GLFW 윈도우
        VkInstance VKinstance;                              // Vulkan 인스턴스
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger;     // 디버그 메신저
        VkPhysicalDevice VKphysicalDevice;                  // 물리 디바이스 -> GPU Physical Handle
        QueueFamilyIndices VKqueueFamilyIndices;            // 큐 패밀리 인덱스 -> VKphysicalDevice에서 선택한 queue family index
        VkDevice VKdevice;                                  // 논리 디바이스 -> GPU Logical Handle
        VkQueue graphicsVKQueue;                            // 그래픽스 큐
        VkQueue presentVKQueue;                             // 프레젠트 큐
        VkSurfaceKHR VKsurface;                             // 서피스 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스
        VkSwapchainKHR VKswapChain;                         // 스왑 체인
        std::vector<VkImage> VKswapChainImages;             // 스왑 체인 이미지 -> 스왑 체인에 사용되는 이미지 Handle
        std::vector<VkImageView> VKswapChainImageViews;       // 스왑 체인 이미지 뷰 -> 스왑 체인 이미지를 뷰로 변환
        VkFormat VKswapChainImageFormat;                    // 스왑 체인 이미지 포맷
        VkExtent2D VKswapChainExtent;                       // 스왑 체인 이미지 해상도
        VkPipelineLayout VKpipelineLayout;                  // 파이프라인 레이아웃
        VkRenderPass VKrenderPass;                          // 렌더 패스
        VkPipeline VKgraphicsPipeline;                        // 그래픽스 파이프라인
        bool state;                                         // 프로그램 상태
    };

    int rateDeviceSuitability(VkPhysicalDevice device);
    void getPyhsicalDeviceProperties(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
}

#endif // INCLUDE_SOURCE_APPLICATION_H