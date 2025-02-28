#include "VKimgui.h"
#include "vkengine.h"

namespace vkengine {
    using vkengine::VulkanEngine;

    namespace gui {
        vkGUI::vkGUI() {}
        vkGUI::~vkGUI() {}
        void vkGUI::init() {
            
            VulkanEngine* engine = &VulkanEngine::Get();

            // Create Descriptor Pool
            VkDescriptorPoolSize pool_sizes[] = {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000;
            pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;

            VkDescriptorPool imguiPool;
            VK_CHECK_RESULT(vkCreateDescriptorPool(engine->getDevice()->VKdevice, &pool_info, nullptr, &imguiPool));

            // init ImGui library
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui_ImplGlfw_InitForVulkan(engine->getWindow(), true);

            // Initialize ImGui
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = engine->getInstance();
            init_info.PhysicalDevice = engine->getDevice()->VKphysicalDevice;
            init_info.Device = engine->getDevice()->VKdevice;
            init_info.QueueFamily = engine->getDevice()->queueFamilyIndices.getGraphicsQueueFamilyIndex();
            init_info.Queue = engine->getDevice()->graphicsVKQueue;
            init_info.DescriptorPool = imguiPool;
            init_info.Allocator = nullptr;
            init_info.MinImageCount = 2;
            init_info.ImageCount = engine->getSwapChain()->getSwapChainImageCount();
            init_info.CheckVkResultFn = nullptr;
            init_info.RenderPass = engine->getRenderPass();
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

            CHECK_RESULT(ImGui_ImplVulkan_Init(&init_info));

            ImGui_ImplVulkan_CreateFontsTexture();
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

            // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

                if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::End();
            }
            
            // Rendering
            ImGui::Render();
        }
    }
}