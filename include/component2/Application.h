#ifndef VK_APPLICATION_3_H_
#define VK_APPLICATION_3_H_

#include "common.h"

#include "VulkanEngineWin2.h"
#include "Camera2.h"
#include "ImGuiRenderer.h"
#include "Model.h"
#include "CommandBufferHandle.h"
#include "Renderer.h"

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
        std::vector<Model> models;
        virtual void updateAnimation(cFloat deltaTime);
        
        cString RootPath{};
        const cUint32_t kMaxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
        const cString AssetsPath = RESOURCE_PATH;
        const cString ShaderPath = SHADER_PATH;
        ShaderManager shaderManager;

        std::shared_ptr<object::Camera2> camera;

        gui::ImGuiRenderer guiRenderer;
        Renderer renderer;

        cUint32_t frameCounter = 0;
        cUint32_t currentFrame = 0;     // For CPU resources (command buffers, fences)
        cUint32_t currentSemaphore = 0; // For GPU semaphores (swapchain sync)

        // NEW: Configuration loading methods
        void initializeWithConfig(const ApplicationConfig &config);
        // void initCamera(const CameraConfig& cameraConfig);
        void loadModels(const std::vector<ModelConfig> &modelConfigs);
        void initCallbacks();
        void initializeVulkanResources();
        void initializeRenderGraph(cString buildRenderGraph);

        void renderHDRControlWindow();
        void renderPostProcessingControlWindow();
        void renderCameraControlWindow();
        void renderSSAOControlWindow();
    };
}

#endif // ! VK_APPLICATION_H_
