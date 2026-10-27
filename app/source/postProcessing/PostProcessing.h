#ifndef POST_PROCESSING_H_
#define POST_PROCESSING_H_

// 홍정모 vulkan 그래픽스 예제 Ex11_PostProcessing 프로젝트 사용

#include "ubo.h"
#include "data.h"

#include "VulkanEngineWin2.h"
#include "ImGuiRenderer.h"
#include "SamplerHandle.h"
#include "UniformBuffer2.h"
#include "DescriptorSetHandle.h"

namespace vkengine {

class PostProcessingExample : public vkengine::VulkanEngineWin2
{
  public:
    PostProcessingExample(std::string root_path, cBool useSwapchain);
    ~PostProcessingExample();

    void runMainLoop();

  private:

    cString RootPath{};
    cString Path = "../../../../../../";

    ShaderManager shaderManager;
    vkengine::gui::ImGuiRenderer guiRenderer;
    std::shared_ptr<vkengine::object::Camera2> camera;

    // Rendering pipelines
    PipelineHandle skyPipeline;
    PipelineHandle postPipeline;

    // Render targets and textures
    VKskyTexture skyTextures;
    Image2D hdrColorBuffer; // HDR color buffer for post-processing input

    SamplerHandle samplerLinearRepeat;
    SamplerHandle samplerLinearClamp;

    // Uniform buffer objects
    SceneDataUBO sceneDataUBO;
    SkyOptionsUBO skyOptionsUBO;
    PostProcessingOptionsUBO postProcessingOptionsUBO;

    // Uniform buffers
    std::vector<UniformBuffer2<SceneDataUBO>> sceneDataUniforms;
    std::vector<UniformBuffer2<SkyOptionsUBO>> skyOptionsUniforms;
    std::vector<UniformBuffer2<PostProcessingOptionsUBO>> postProcessingOptionsUniforms;

    // Descriptor sets
    std::vector<DescriptorSetHandle> sceneDescriptorSets;
    std::vector<DescriptorSetHandle> postProcessingDescriptorSets;
    DescriptorSetHandle skyDescriptorSet;

    // Methods
    void initializeSkybox();
    void initializePostProcessing();
    void renderFrame();
    void updateGui(VkExtent2D windowSize);
    void renderHDRControlWindow();
    void renderPostProcessingControlWindow();

    void recordCommandBuffer(CommandBufferHandle& cmd, uint32_t imageIndex, VkExtent2D windowSize);
    void submitFrame(CommandBufferHandle& commandBuffer, VkSemaphore waitSemaphore,
                     VkSemaphore signalSemCommandBufferaphore, VkFence fence);
    virtual void recreateSwapchain();

};

}

#endif // !POST_PROCESSING_H_
