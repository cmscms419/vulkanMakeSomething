#ifndef VK_PIPELIN_INCLUDE_H_
#define VK_PIPELIN_INCLUDE_H_

#include "common.h"
#include "vkconfig.h"
#include <optional>

#include "Context.h"
#include "ShaderManager.h"
#include "DescriptorSetHandle.h"

namespace vkengine
{
    class PipelineHandle
    {
    public:
        PipelineHandle(Context &context, ShaderManager &shaderManager);
        PipelineHandle(Context &ctx, ShaderManager &shaderManager, cString Name, VkFormat outColorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamples);
        PipelineHandle(PipelineHandle &&other) noexcept;
        PipelineHandle &operator=(PipelineHandle &&other) noexcept;
        PipelineHandle(const PipelineHandle &) = delete;
        PipelineHandle &operator=(const PipelineHandle &) = delete;
        PipelineHandle(Context &ctx, ShaderManager &shaderManager,
                         const PipelineConfig &pipelineCache,
                         std::vector<VkFormat> outColorFormats = {},
                         std::optional<VkFormat> depthFormat = std::nullopt,
                         std::optional<VkSampleCountFlagBits> msaaSamples = std::nullopt);

        ~PipelineHandle();
        void cleanup();

        // 공통으로 사용되는 파이프라인 설정을 여기에 추가할 수 있습니다.
        void createCommon();
        void createComputePipeline();

        // config에서 파이프라인 이름으로 구분하여 파이프라인을 생성하는 함수
        void createFromConfig(
            const PipelineConfig &config,
            std::vector<VkFormat> outColorFormats = {},
            std::optional<VkFormat> depthFormat = std::nullopt,
            std::optional<VkSampleCountFlagBits> msaaSamples = std::nullopt);
        
        void createGraphicsFromConfig(
            const PipelineConfig &config,
            std::vector<VkFormat> outColorFormats,
            std::optional<VkFormat> depthFormat,
            std::optional<VkSampleCountFlagBits> msaaSamples);

        void createByName(cString name,
                          std::optional<VkFormat> outColorFormat = VK_FORMAT_UNDEFINED,
                          std::optional<VkFormat> depthFormat = VK_FORMAT_UNDEFINED,
                          std::optional<VkSampleCountFlagBits> msaaSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM);

        void createSquarePipeline(
            VkFormat outColorFormat = VK_FORMAT_UNDEFINED,
            VkFormat depthFormat = VK_FORMAT_UNDEFINED,
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM);

        void createSkyboxPipeline(
            VkFormat outColorFormat,
            VkFormat depthFormat,
            VkSampleCountFlagBits msaaSamples);

        void createGuiPipeline(VkFormat outColorFormat = VK_FORMAT_UNDEFINED);

        void createPostProcessingPipeline(
            VkFormat outColorFormat = VK_FORMAT_UNDEFINED,
            VkFormat depthFormat = VK_FORMAT_UNDEFINED,
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM);

        void createShadowMapPipeline(
            VkFormat depthFormat = VK_FORMAT_UNDEFINED);

        void createForwardPBRPipeline(
            VkFormat outColorFormat,
            VkFormat depthFormat,
            VkSampleCountFlagBits msaaSamples);

        void createDeferredPBRPipeline(
            VkFormat outColorFormat,
            VkFormat depthFormat,
            VkSampleCountFlagBits msaaSamples);

        void dispatch(const VkCommandBuffer &cmd, uint32_t frameIndex);

        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }
        VkPipeline getPipeline() { return pipeline; }

    private:
        Context &ctx;
        ShaderManager &shaderManager;

        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;

        cString name;
        // std::vector<VkDescriptorSetLayout> layouts_{};
        // std::vector<std::vector<std::reference_wrapper<DescriptorSetHandle>>> descriptorSets; // [frameNumber][setIndex]
        // std::vector<std::vector<VkDescriptorSet>> descriptorSetHandles; // [frameNumber][setIndex]
        // std::vector<std::vector<BindingInfo>> bindingInfos;
    };
}

#endif // !VK_PIPELIN_INCLUDE_H_
