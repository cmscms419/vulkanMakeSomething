#ifndef INCLUDE_SOURCE_APPLICATION_H
#define INCLUDE_SOURCE_APPLICATION_H

#define VKUTIL_APPLICATION_H

#include <cms491_common.h>

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

        // 도구
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        bool isDeviceSuitable(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
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
        bool state;                                         // 프로그램 상태
    };

    int rateDeviceSuitability(VkPhysicalDevice device);
    void getPyhsicalDeviceProperties(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
}

#endif // INCLUDE_SOURCE_APPLICATION_H