#ifndef INCLUDE_VK_ENGINE_2_
#define INCLUDE_VK_ENGINE_2_


#include "VKContext.h"
#include "VKwindow.h"

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
        cBool framebufferResized{ false };   // ������ ���� ũ�� ���� ����

    private:

        void initWindow();
        void initVulkan();

        std::string RootPath{};

        std::unique_ptr<platform::VirtualWindows> window;
        
        VkContext cxt;

        

    };
}


#endif // !INCLUDE_VK_ENGINE_2_
