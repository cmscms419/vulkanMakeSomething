#ifndef POST_PROCESSING_H_
#define POST_PROCESSING_H_

// 홍정모 vulkan 그래픽스 예제 Ex11_PostProcessing 프로젝트 사용


#include "VKengine2.h"
#include "VKgui.h"
#include "VKsampler.h"
#include "VKUniformBuffer2.h"
#include "DescriptorSet.h"

namespace vkengine {

class PostProcessingExample : public vkengine::VulkanEngineWin2
{
  public:
    PostProcessingExample(std::string root_path, cBool useSwapchain);
    ~PostProcessingExample();

    void mainLoop();

  private:

    VKShaderManager shaderManager;
    vkengine::gui::VKimguiRenderer guiRenderer;
    std::shared_ptr<vkengine::object::Camera2> camera;

    // Rendering pipelines
    PipeLineHandle skyPipeline;
    PipeLineHandle postPipeline;

    // Render targets and textures
    VKskyTexture skyTextures;
    VKImage2D hdrColorBuffer; // HDR color buffer for post-processing input

    VKSampler samplerLinearRepeat;
    VKSampler samplerLinearClamp;

    // Uniform buffer objects
    SceneDataUBO sceneDataUBO;
    SkyOptionsUBO skyOptionsUBO;
    PostProcessingOptionsUBO postProcessingOptionsUBO;

    // Uniform buffers
    std::vector<VKUniformBuffer2<SceneDataUBO>> sceneDataUniforms;
    std::vector<VKUniformBuffer2<SkyOptionsUBO>> skyOptionsUniforms;
    std::vector<VKUniformBuffer2<PostProcessingOptionsUBO>> postProcessingOptionsUniforms;

    // Descriptor sets
    std::vector<DescriptorSetHander> sceneDescriptorSets;
    std::vector<DescriptorSetHander> postProcessingDescriptorSets;
    DescriptorSetHander skyDescriptorSet;

    // Methods
    void initializeSkybox();
    void initializePostProcessing();
    void renderFrame();
    void updateGui(VkExtent2D windowSize);
    void renderHDRControlWindow();
    void renderPostProcessingControlWindow();

    void recordCommandBuffer(VKCommandBufferHander& cmd, uint32_t imageIndex, VkExtent2D windowSize);
    void submitFrame(VKCommandBufferHander& commandBuffer, VkSemaphore waitSemaphore,
                     VkSemaphore signalSemCommandBufferaphore, VkFence fence);
    virtual void recreateSwapchain();

};

}

#endif // !POST_PROCESSING_H_