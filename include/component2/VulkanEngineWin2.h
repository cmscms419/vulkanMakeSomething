#ifndef INCLUDE_VK_ENGINE_2_
#define INCLUDE_VK_ENGINE_2_

#include <chrono>

#include "Context.h"
#include "Image2D.h"
#include "ShaderManager.h"
#include "Shader.h"
#include "PipelineHandle.h"
#include "BarrierHelper.h"
#include "CommandBufferHandle.h"
#include "BaseBuffer2.h"
#include "SwapChain.h"
#include "UniformBuffer2.h"
#include "DescriptorSetHandle.h"

#include "Camera2.h"
#include "Window.h"

namespace vkengine {

    class VulkanEngineWin2
    {
    public:
        VulkanEngineWin2(cBool useSwapchain);
        ~VulkanEngineWin2();
        void init();
        void cleanup();

        cBool _isInitialized;
        cBool stop_rendering;
        cBool framebufferResized;   // 프레임 버퍼 크기 조정 여부

    protected:

        void calculateCurrentValue();
        virtual void recreateSwapchain();

        // Core Vulkan objects
        std::shared_ptr<Context> cxt;
        std::shared_ptr<SwapChain> swapChain;
        std::unique_ptr<platform::VirtualWindows> window;

        // Frame resources
        std::vector<CommandBufferHandle> commandBuffers;
        std::vector<VkSemaphore> presentSemaphores;
        std::vector<VkSemaphore> renderSemaphores;
        std::vector<VkFence> inFlightFences;

        VkExtent2D extent;
        platform::WindowCreateInfo createInfo;
        std::shared_ptr<object::Camera2> camera;

        bool shouldClose;

        cUint32_t currentFrame;
        cUint32_t currentSemaphore;
    };
}

#endif // !INCLUDE_VK_ENGINE_2_
