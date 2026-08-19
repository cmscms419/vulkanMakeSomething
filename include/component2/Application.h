#ifndef VK_APPLICATION_3_H_
#define VK_APPLICATION_3_H_

#include "common.h"

#include "VKengine2.h"
#include "Camera2.h"
#include "VKgui.h"
#include "VKModel.h"
#include "VKCommandBufferHander.h"
#include "VKrenderer.h"

namespace vkengine {

    class Application : public VulkanEngineWin2
    {
    public:
        Application(cString root_path);                                 // Default configuration
        Application(const ApplicationConfig& config, cString root_path);  // Custom configuration
        Application(const cString& configFile, cString root_path);        // Load from file (future feature)
        ~Application();

        virtual void run();
        virtual void update();
        virtual void updateGui();

    protected:
        std::vector<VKModel> models;
        virtual void updateAnimation(cFloat deltaTime);
        
        cString RootPath{};
        const cUint32_t kMaxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
        const cString AssetsPath = RESOURSE_PATH;
        const cString ShaderPath = SHADER_PATH;
        VKShaderManager shaderManager;

        std::shared_ptr<object::Camera2> camera;

        gui::VKimguiRenderer guiRenderer;
        VKRenderer Renderer;

        cUint32_t frameCounter = 0;
        cUint32_t currentFrame = 0;     // For CPU resources (command buffers, fences)
        cUint32_t currentSemaphore = 0; // For GPU semaphores (swapchain sync)

        // NEW: Configuration loading methods
        void initializeWithConfig(const ApplicationConfig &config);
        // void setupCamera(const CameraConfig& cameraConfig);
        void loadModels(const std::vector<ModelConfig> &modelConfigs);
        void setupCallbacks();
        void initializeVulkanResources();
        void initializeRenderGraph(cString buildRenderGraph);

        void renderHDRControlWindow();
        void renderPostProcessingControlWindow();
        void renderCameraControlWindow();
        void renderSSAOControlWindow();
    };
}

#endif // ! VK_APPLICATION_H_