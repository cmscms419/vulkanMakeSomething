#include "VKengine2.h"

namespace vkengine
{
    VulkanEngineWin2* loadedEngine = nullptr;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanEngineWin2*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
    
    VulkanEngineWin2::VulkanEngineWin2(cBool useSwapchain)
    {
        this->window = platform::WindowFactory::create(
            platform::WindowFactory::Backend::GLFW,
            this->createInfo);

        int windowWidth = 0;
        int windowHeight = 0;
        window->getFramebufferSize(&windowWidth, &windowHeight);

        this->extent.width = static_cast<cUint32_t>(windowWidth);
        this->extent.height = static_cast<cUint32_t>(windowHeight);
        this->cxt = std::make_shared<VKcontext>(this->window->getRequiredExtensions(), true);
        this->swapChain = std::make_shared<VKSwapChain>(*this->cxt, window->createVulkanSurface(this->cxt->getInstance()), extent, true);

        helper::initializeSynchronization(
            this->cxt->getDevice()->logicaldevice,
            MAX_FRAMES_IN_FLIGHT, // 최대 개수를 2개로 지정
            this->swapChain->getSwapChainImageCount(),
            this->presentSemaphores,
            this->renderSemaphores,
            this->inFlightFences
        );

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

            for (auto& commandbuffer : this->commandBuffers) {
                commandbuffer.cleanup();
            }

            this->swapChain.reset();
            this->cxt.reset();

        }
    }

    void VulkanEngineWin2::caluCurrentValue()
    {
        cUint32_t imageCount = this->swapChain->getSwapChainImageCount();

        this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        this->currentSemaphore = (this->currentSemaphore + 1) % imageCount;
    }

    void VulkanEngineWin2::recreateSwapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(this->window->getGLFWwindow(), &width, &height);

        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(this->window->getGLFWwindow(), &width, &height);
            glfwWaitEvents();
        }

        this->extent.width = static_cast<cUint32_t>(width);
        this->extent.height = static_cast<cUint32_t>(height);

        vkDeviceWaitIdle(this->cxt->getDevice()->logicaldevice);

        this->cxt->getDepthStencil()->cleanup(this->cxt->getDevice()->logicaldevice);
        
        this->swapChain->cleanupWithoutSurface();
        this->swapChain->initSurface(this->swapChain->Surface(), true);
        this->swapChain->createSwapChain();

        // 8. Depth/Stencil 리소스 재생성 (필요한 경우)
        if (this->cxt->getUSEdepthStencil()) {

            this->cxt->getDepthStencil()->cleanup(this->cxt->getDevice()->logicaldevice);
            VkFormat depthFormat = this->cxt->getDepthStencil()->depthFormat;

            // Depth image 재생성
            vkengine::helper::createImage2(
                this->cxt->getDevice()->logicaldevice,
                this->cxt->getDevice()->physicalDevice,
                this->extent.width,
                this->extent.height,
                1,  // mipLevels
                VK_SAMPLE_COUNT_1_BIT,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                this->cxt->getDepthStencil()->depthImage,
                this->cxt->getDepthStencil()->depthImageMemory,
                1,  // arrayLayers
                (VkImageCreateFlagBits)0
            );

            // Depth image view 재생성
            this->cxt->getDepthStencil()->depthImageView = vkengine::helper::createImageView(
                this->cxt->getDevice()->logicaldevice,
                this->cxt->getDepthStencil()->depthImage,
                depthFormat,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                1  // mipLevels
            );
        }

    }

}