#include "VKimgui.h"
#include "vkengine.h"
#include "helper.h"

#include "imgui.h" 
#include "imconfig.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imstb_rectpack.h"
#include "imstb_textedit.h"
#include "imstb_truetype.h"

namespace vkengine {
    using vkengine::VulkanEngine;
    using namespace helper;

    namespace gui {
        vkGUI::vkGUI() {}
        vkGUI::~vkGUI() {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
        void vkGUI::init() {
            
            VulkanEngine* engine = &VulkanEngine::Get();

            // init ImGui library
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            ImGui_ImplGlfw_InitForVulkan(engine->getWindow(), true);

            // Initialize ImGui
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = engine->getInstance();
            init_info.PhysicalDevice = engine->getDevice()->VKphysicalDevice;
            init_info.Device = engine->getDevice()->VKdevice;
            init_info.QueueFamily = engine->getDevice()->queueFamilyIndices.getGraphicsQueueFamilyIndex();
            init_info.Queue = engine->getDevice()->graphicsVKQueue;
            init_info.DescriptorPool = engine->getDescriptorPool();
            init_info.Allocator = nullptr;
            init_info.MinImageCount = 2;
            init_info.ImageCount = engine->getSwapChain()->getSwapChainImageCount();
            init_info.CheckVkResultFn = nullptr;
            init_info.RenderPass = engine->getRenderPass();
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

            CHECK_RESULT(ImGui_ImplVulkan_Init(&init_info));

            ImGui_ImplVulkan_CreateFontsTexture();

            vkDeviceWaitIdle(engine->getDevice()->VKdevice);

        }
        void vkGUI::initResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath)
        {

        }
        void vkGUI::update()
        {
            // Start the Dear ImGui frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();

            ImGui::NewFrame();

            ImGui::ShowDemoWindow();

            // Rendering
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanEngine::Get().getCurrnetFrameData().mainCommandBuffer);

            //ImDrawData* draw_data = ImGui::GetDrawData();
            //const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
            //if (!is_minimized)
            {
                //wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
                //wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
                //wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
                //wd->ClearValue.color.float32[3] = clear_color.w;
                //FrameRender(wd, draw_data);
                //FramePresent(wd);
            }
        }
    }
}