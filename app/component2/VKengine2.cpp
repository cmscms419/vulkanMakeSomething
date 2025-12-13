#include "VKengine2.h"

namespace vkengine
{
    constexpr cBool bUseValidationLayers = false;
    VulkanEngineWin2* loadedEngine = nullptr;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanEngineWin2*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
    VulkanEngineWin2::VulkanEngineWin2(std::string root_path, const std::vector<const char*>& requiredInstanceExtensions, cBool useSwapchain)
        : RootPath(root_path), cxt(requiredInstanceExtensions, useSwapchain)
    {
        initWindow();
        
        _isInitialized = true;
    }
    VulkanEngineWin2::~VulkanEngineWin2()
    {
        cleanup();
    }
    void VulkanEngineWin2::init()
    {
        assert(loadedEngine == nullptr);
        loadedEngine = this;

    }
    void VulkanEngineWin2::cleanup()
    {
        if (this->_isInitialized)
        {

        }
    }
    void VulkanEngineWin2::initWindow()
    {
        
    }

    void VulkanEngineWin2::initVulkan()
    {
    }

}