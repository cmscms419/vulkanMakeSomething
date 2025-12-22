#include "pipeLineHandle.h"

using namespace vkengine::Log;

namespace vkengine {

    void PipeLineHandle::cleanup()
    {
        if (pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(ctx.getDevice()->logicaldevice, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(ctx.getDevice()->logicaldevice, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }
    }

    void PipeLineHandle::createCommon()
    {
        cleanup();

        std::vector<VkDescriptorSetLayout> layouts = this->ctx.getDescriptorManager()->getLayoutsForPipeline(this->name);
        VkPushConstantRange pushConstantRange = this->shaderManager.pushConstantsRange(this->name);

        VkPipelineLayoutCreateInfo pipelineLayoutCI = helper::pipelineLayoutCreateInfo(layouts.data(), static_cast<cUint32_t>(layouts.size()));

        if (pushConstantRange.size > 0) {
            pipelineLayoutCI.pushConstantRangeCount = 1;
            pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
        }
        else {
            pipelineLayoutCI.pushConstantRangeCount = 0;
            pipelineLayoutCI.pPushConstantRanges = nullptr;
        }
        _VK_CHECK_RESULT_(vkCreatePipelineLayout(ctx.getDevice()->logicaldevice, &pipelineLayoutCI, nullptr, &pipelineLayout));
    }

    void PipeLineHandle::createByName(
        cString name,
        std::optional<VkFormat> outColorFormat,
        std::optional<VkFormat> depthFormat,
        std::optional<VkSampleCountFlagBits> msaaSamples)
    {
        this->name = name;

        createCommon();

        if (name == "sample_pipeline") {
            createSquarePipeline(
                outColorFormat.value(),
                depthFormat.value(),
                msaaSamples.value()
            );
        }
        else if (name == "gui")
        {
            createGuiPipeline(outColorFormat.value());
        }
        else if (name == "sky")
        {
            if (outColorFormat.has_value() && depthFormat.has_value() && msaaSamples.has_value())
            {
                this->createSkyboxPipeline(
                    outColorFormat.value(),
                    depthFormat.value(),
                    msaaSamples.value()
                );
            }
            else
            {
                Log::EXIT_TO_LOGGER("outColorFormat, depthFormat, and msaaSamples required for %s", this->name);
            }
        }
        else if (name == "post")
        {
            this->createPostProcessingPipeLine
            (
                outColorFormat.value(),
                depthFormat.value(),
                msaaSamples.value()
            );
        }
        else {
            PRINT_TO_LOGGER("Error: Unknown pipeline name: " + name);
        }

    }

}