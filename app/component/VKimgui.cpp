#include "VKimgui.h"

#include "Camera.h"

namespace vkengine
{
    vkGUI::vkGUI(VulkanEngine* engine) : engine(engine) {}

    vkGUI::~vkGUI() {
        
    }

    void vkGUI::init(float width, float height) {
        engine->createDescriptorPoolImGui();
    }

    void vkGUI::initResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath)
    {
        // init ImGui library
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui_ImplGlfw_InitForVulkan(engine->getWindow(), true);

        // Initialize ImGui
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = engine->getInstance();
        init_info.PhysicalDevice = engine->getDevice()->physicalDevice;
        init_info.Device = engine->getDevice()->logicaldevice;
        init_info.QueueFamily = engine->getDevice()->queueFamilyIndices.getGraphicsQueueFamilyIndex();
        init_info.Queue = engine->getDevice()->graphicsVKQueue;
        init_info.DescriptorPool = engine->getDescriptorPool();
        init_info.MinImageCount = 2;
        init_info.ImageCount = engine->getSwapChain()->getSwapChainImageCount();
        init_info.CheckVkResultFn = VK_NULL_HANDLE;
        init_info.RenderPass = engine->getRenderPass();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.Allocator = VK_NULL_HANDLE;

        _CHECK_RESULT_(ImGui_ImplVulkan_Init(&init_info));
        ImGui_ImplVulkan_CreateFontsTexture();
        ImGui_ImplVulkan_DestroyFontsTexture();
    }
    
    void vkGUI::update()
    {
        // demo window
        //ImGui::ShowDemoWindow();
        ImGui::Text("Hello, world!");
        // Access the camera position from the engine
        std::shared_ptr<vkengine::object::Camera> camera = engine->getCamera();
        glm::vec3 position = camera->getPos();

        // Display the camera position using ImGui
        ImGui::Text("Camera Position:");
        ImGui::Text("X: %.4f, Y: %.4f, Z: %.4f", position.x, position.y, position.z);

#if DEBUG_

        ImGui::Text("Program Run Time: %.2f ms", this->engine->getProgramRunTime());
        ImGui::Text("current delta Time: %.5f ms", this->engine->getCalulateDeltaTime());

#endif // DEBUG

    }
    void vkengine::vkGUI::begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        ImGui::Begin("Scene Control");
    }

    void vkengine::vkGUI::end()
    {
        ImGui::End();
    }
    void vkGUI::render()
    {
        ImGui::Render();
        
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanEngine::Get().getCurrnetFrameData().mainCommandBuffer);

        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    }
    void vkGUI::cleanup()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}