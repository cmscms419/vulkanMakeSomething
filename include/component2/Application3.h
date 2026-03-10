#ifndef VK_APPLICATION_3_H_
#define VK_APPLICATION_3_H_

#include "common.h"

#include "VKengine2.h"
#include "Camera2.h"
#include "VKgui.h"
#include "VKModel.h"
#include "VKCommandBufferHander.h"
#include "VKrenderer2.h"

namespace vkengine {

    class Application3 : public VulkanEngineWin2
    {
    public:
        Application3(cString root_path);                                 // Default configuration
        Application3(const ApplicationConfig& config, cString root_path);  // Custom configuration
        Application3(const cString& configFile, cString root_path);        // Load from file (future feature)
        ~Application3();

        void run();
        void update();
        void updateGui();

    private:
        cString RootPath{};
        const cUint32_t kMaxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
        const cString AssetsPath = RESOURSE_PATH;
        const cString ShaderPath = SHADER_PATH;

        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;

        std::shared_ptr<object::Camera2> camera;
        std::vector<VKModel> models;

        gui::VKimguiRenderer guiRenderer;
        VKShaderManager shaderManager;
        VKforwardRenderer2 forwardRenderer;

        cUint32_t frameCounter = 0;
        cUint32_t currentFrame = 0;     // For CPU resources (command buffers, fences)
        cUint32_t currentSemaphore = 0; // For GPU semaphores (swapchain sync)

        // NEW: Configuration loading methods
        void initializeWithConfig(const ApplicationConfig& config);
        //void setupCamera(const CameraConfig& cameraConfig);
        void loadModels(const std::vector<ModelConfig>& modelConfigs);
        void setupCallbacks();
        void initializeVulkanResources();
        void initializeRenderGraph();

        void renderHDRControlWindow();
        void renderPostProcessingControlWindow();
        void renderCameraControlWindow();
    };
}

#endif // ! VK_APPLICATION_H_