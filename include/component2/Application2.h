#ifndef VK_APPLICATION_2_H_
#define VK_APPLICATION_2_H_

#include "common.h"
#include "helper.h"
#include "log.h"

#include "VKengine2.h"
#include "Camera2.h"
#include "VKgui.h"
#include "VKModel.h"
#include "VKCommadBufferHander.h"
#include "VKrenderer.h"

namespace vkengine {

    class Application2 : public VulkanEngineWin2
    {
    public:
        Application2(cString root_path);                                 // Default configuration
        Application2(const ApplicationConfig& config, cString root_path);  // Custom configuration
        Application2(const cString& configFile, cString root_path);        // Load from file (future feature)
        ~Application2();

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

        VKShaderManager shaderManager;

        std::vector<VKModel> models{};

        gui::VKimguiRenderer guiRenderer;
        VKforwardRenderer forwardRenderer;

        cUint32_t frameCounter = 0;
        cUint32_t currentFrame = 0;     // For CPU resources (command buffers, fences)
        cUint32_t currentSemaphore = 0; // For GPU semaphores (swapchain sync)

        // NEW: Configuration loading methods
        void initializeWithConfig(const ApplicationConfig& config);
        //void setupCamera(const CameraConfig& cameraConfig);
        void loadModels(const std::vector<ModelConfig>& modelConfigs);
        void setupCallbacks();
        void initializeVulkanResources();

        void renderHDRControlWindow();
        void renderPostProcessingControlWindow();
        void renderCameraControlWindow();
    };
}

#endif // ! VK_APPLICATION_H_
