#include "VKengine.h"
#include "VKkey.h"

using vkcms::input::key_callback;

namespace vkcms
{
    constexpr bool bUseValidationLayers = false;
    VulkanEngine* loadedEngine = nullptr;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    VulkanEngine& VulkanEngine::Get() { return *loadedEngine; }

    void VulkanEngine::init()
    {
        assert(loadedEngine == nullptr);
        loadedEngine = this;

        this->initWindow();
    }

    void VulkanEngine::cleanup()
    {
        if (this->_isInitialized)
        {
            glfwDestroyWindow(this->VKwindow);
            glfwTerminate();
        }

        loadedEngine = nullptr;
    }

    bool VulkanEngine::mainLoop()
    {
        while (!glfwWindowShouldClose(this->VKwindow)) {
            glfwPollEvents();
            drawFrame();
#ifdef DEBUG_
            //printf("update\n");
#endif // DEBUG_

        }

        //vkDeviceWaitIdle(this->VKdevice);
        state = false;

        return state;
    }

    void VulkanEngine::drawFrame()
    {
    }

    void VulkanEngine::initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        int WIDTH_ = WIDTH;
        int HEIGHT_ = HEIGHT;

        this->VKwindow = glfwCreateWindow(WIDTH_, HEIGHT_, "Vulkan Test", nullptr, nullptr);

        glfwSetWindowUserPointer(this->VKwindow, this);
        glfwSetFramebufferSizeCallback(this->VKwindow, framebufferResizeCallback);
        glfwSetKeyCallback(this->VKwindow, vkcms::input::key_callback);  // 키 입력 콜백 설정

        this->_isInitialized = true;
    }

    void VulkanEngine::init_vulkan()
    {

    }

    void VulkanEngine::init_swapchain()
    {
    }

    void VulkanEngine::init_commands()
    {
    }

    void VulkanEngine::init_sync_structures()
    {
    }

}


