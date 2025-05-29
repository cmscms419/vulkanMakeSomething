#ifndef INCLUDE_IMGUI_H_
#define INCLUDE_IMGUI_H_

#include "common.h"

#include "VKengine.h"

#include "imgui.h" 
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace vkengine
{
    class vkGUI {
    public:
        vkGUI(VulkanEngine* engine);
        ~vkGUI();
        void init(float width, float height);
        void initResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath);
        void update();
        void begin();
        void end();
        void render();
        void cleanup();
    private:
        VulkanEngine* engine;
    };
}

#endif // INCLUDE_IMGUI_H_