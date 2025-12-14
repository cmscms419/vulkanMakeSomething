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
        : RootPath(root_path)
    {
        initWindow();
        initVulkan(requiredInstanceExtensions, useSwapchain);
        
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
            this->cxt->waitIdle();

            for (auto& semaphore : this->presentSemaphores) {
                vkDestroySemaphore(this->cxt->getDevice()->logicaldevice, semaphore, nullptr);
            }
            for (auto& semaphore : this->renderSemaphores) {
                vkDestroySemaphore(this->cxt->getDevice()->logicaldevice, semaphore, nullptr);
            }
            for (auto& fence : this->inFlightFences) {
                vkDestroyFence(this->cxt->getDevice()->logicaldevice, fence, nullptr);
            }
        }
    }
    void VulkanEngineWin2::initWindow()
    {
        this->window = platform::WindowFactory::create(
            platform::WindowFactory::Backend::GLFW,
            this->createInfo,
            this->camera);

        int windowWidth = 0;
        int windowHeight = 0;
        window->getFramebufferSize(&windowWidth, &windowHeight);

        this->extent.width = static_cast<cUint32_t>(windowWidth);
        this->extent.height = static_cast<cUint32_t>(windowHeight);
    }

    void VulkanEngineWin2::initVulkan(const std::vector<const char*>& requiredInstanceExtensions, cBool useSwapchain)
    {
        this->cxt = std::make_shared<VKcontext>(requiredInstanceExtensions, useSwapchain);
        this->swapChain = std::make_shared<VKSwapChain>(this->cxt, window->createVulkanSurface(this->cxt->getInstance()), extent, true);
        helper::initializeSynchronization(
            this->cxt->getDevice()->logicaldevice,
            MAX_FRAMES_IN_FLIGHT, // 최대 개수를 2개로 지정
            this->swapChain->getSwapChainImageCount(),
            this->presentSemaphores,
            this->renderSemaphores,
            this->inFlightFences
        );

    }

    void VulkanEngineWin2::caluCurrentValue()
    {
        cUint32_t imageCount = this->swapChain->getSwapChainImageCount();

        this->currentFrame = (this->currentFrame + 1) % imageCount;
        this->currentSemaphore = (this->currentSemaphore + 1) % imageCount;
    }

}