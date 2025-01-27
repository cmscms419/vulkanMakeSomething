#ifndef INCLUDE_SOURCE_APPLICATION_H
#define INCLUDE_SOURCE_APPLICATION_H

#define VKUTIL_APPLICATION_H

#include <cms491_common.h>

constexpr int MAX_FRAMES = 4;

constexpr auto shaderRootPath = "shader/";

namespace vkutil {

    class Application {
    public:

        virtual void init();
        virtual bool run();
        virtual void update();
        virtual void setup();
        virtual void cleanup();


    private:
        virtual void initWindow();
        virtual void initVulkan();
        virtual void mainLoop();
        
        void createInstance();
        void setupDebugCallback();
        void pickPhysicalDevice();
        void createLogicalDevice();

        GLFWwindow* VKwindow = nullptr;                     // GLFW 윈도우
        VkInstance VKinstance = {};                         // Vulkan 인스턴스
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger;     // 디버그 메신저
        VkPhysicalDevice VKphysicalDevice = VK_NULL_HANDLE; // 물리 디바이스 -> GPU Physical Handle
        QueueFamilyIndices VKqueueFamilyIndices = {};       // 큐 패밀리 인덱스
        VkDevice VKdevice = VK_NULL_HANDLE;                 // 논리 디바이스 -> GPU Logical Handle
        VkQueue VKQueue;                                    // 큐
        bool state = true; // 프로그램 상태
    };

    int rateDeviceSuitability(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    void getPyhsicalDeviceProperties(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkQueueFlagBits bit);
    bool isDeviceSuitable(VkPhysicalDevice device);
}

#endif // INCLUDE_SOURCE_APPLICATION_H