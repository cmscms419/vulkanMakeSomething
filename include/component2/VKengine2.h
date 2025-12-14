#ifndef INCLUDE_VK_ENGINE_2_
#define INCLUDE_VK_ENGINE_2_

#include <chrono>

#include "log.h"

#include "VKContext.h"
#include "VKImage2D.h"
#include "VKShaderManager.h"
#include "VKshader.h"
#include "pipeLineHandle.h"
#include "VKbarrier2.h"
#include "VKCommadBufferHander.h"
#include "VKbuffer2.h"
#include "VKswapchain.h"
#include "VKSkyTexture.h"
#include "VKUniformBuffer2.h"
#include "DescriptorSet.h"

#include "Camera2.h"
#include "VKwindow.h"


using namespace vkengine;
using namespace vkengine::Log;

using namespace vkengine;

namespace vkengine {
    class VulkanEngineWin2
    {
    public:
        VulkanEngineWin2(std::string root_path, const std::vector<const char*>& requiredInstanceExtensions, cBool useSwapchain);
        ~VulkanEngineWin2();
        void init();
        void cleanup();

        cBool _isInitialized{ false };
        cBool stop_rendering{ false };
        cBool framebufferResized{ false };   // 프레임 버퍼 크기 조정 여부

    protected:

        virtual void initWindow();
        virtual void initVulkan(const std::vector<const char*>& requiredInstanceExtensions, cBool useSwapchain);
        void caluCurrentValue();

        std::string RootPath{};
        std::unique_ptr<platform::VirtualWindows> window;
        
        // Core Vulkan objects
        std::shared_ptr<VKcontext> cxt;
        std::shared_ptr<VKSwapChain> swapChain;
        std::shared_ptr<VKShaderManager> shaderManager;

        // Frame resources
        std::vector<VKCommandBufferHander> commandBuffers;
        std::vector<VkSemaphore> presentSemaphores;
        std::vector<VkSemaphore> renderSemaphores;
        std::vector<VkFence> inFlightFences;

        VkExtent2D extent;
        platform::WindowCreateInfo createInfo;
        std::shared_ptr<object::Camera2> camera;

        bool shouldClose{ false };

        cUint32_t currentFrame = 0;
        cUint32_t currentSemaphore = 0;
    };
}


#endif // !INCLUDE_VK_ENGINE_2_
