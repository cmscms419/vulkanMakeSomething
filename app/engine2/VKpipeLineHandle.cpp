#include "VKpipeLineHandle.h"

#include "log.h"
#include "helper.h"

using namespace vkengine::Log;

namespace vkengine
{
    VKPipeLineHandle::VKPipeLineHandle(VKcontext &context, VKShaderManager &shaderManager) : ctx(context), shaderManager(shaderManager)
    {
        pipelineLayout = VK_NULL_HANDLE;
        pipeline = VK_NULL_HANDLE;
    }

    VKPipeLineHandle::VKPipeLineHandle(VKPipeLineHandle &&other) noexcept : ctx(other.ctx), shaderManager(other.shaderManager), pipelineLayout(other.pipelineLayout), pipeline(other.pipeline), name(std::move(other.name))
    {
        other.pipelineLayout = VK_NULL_HANDLE;
        other.pipeline = VK_NULL_HANDLE;
    }

    VKPipeLineHandle &VKPipeLineHandle::operator=(VKPipeLineHandle &&other) noexcept
    {
        if (this != &other)
        {
            cleanup();

            pipelineLayout = other.pipelineLayout;
            pipeline = other.pipeline;
            name = std::move(other.name);
            other.pipelineLayout = VK_NULL_HANDLE;
            other.pipeline = VK_NULL_HANDLE;
        }
        return *this;
    }

    VKPipeLineHandle::~VKPipeLineHandle()
    {
        cleanup();
    }

    void VKPipeLineHandle::cleanup()
    {
        if (pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(ctx.getDevice()->logicaldevice, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }
        if (pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(ctx.getDevice()->logicaldevice, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }
    }

    void VKPipeLineHandle::createCommon()
    {
        cleanup();

        std::vector<VkDescriptorSetLayout> layouts = this->ctx.getDescriptorManager()->getLayoutsForPipeline(this->name);
        VkPushConstantRange pushConstantRange = this->shaderManager.pushConstantsRange(this->name);

        VkPipelineLayoutCreateInfo pipelineLayoutCI = helper::pipeline::pipelineLayoutCreateInfo(layouts.data(), static_cast<cUint32_t>(layouts.size()));

        if (pushConstantRange.size > 0)
        {
            pipelineLayoutCI.pushConstantRangeCount = 1;
            pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
        }
        else
        {
            pipelineLayoutCI.pushConstantRangeCount = 0;
            pipelineLayoutCI.pPushConstantRanges = nullptr;
        }
        _VK_CHECK_RESULT_(vkCreatePipelineLayout(ctx.getDevice()->logicaldevice, &pipelineLayoutCI, nullptr, &pipelineLayout));
    }

    // void VKPipeLineHandle::createByName(
    //     cString name,
    //     std::optional<VkFormat> outColorFormat,
    //     std::optional<VkFormat> depthFormat,
    //     std::optional<VkSampleCountFlagBits> msaaSamples)
    // {
    //     this->name = name;

    //     createCommon();

    //     if (name == "sample_pipeline")
    //     {
    //         createSquarePipeline(outColorFormat.value(), depthFormat.value(), msaaSamples.value());
    //     }
    //     else if (name == "gui")
    //     {
    //         createGuiPipeline(outColorFormat.value());
    //     }
    //     else if (name == "sky")
    //     {
    //         if (outColorFormat.has_value() && depthFormat.has_value() && msaaSamples.has_value())
    //         {
    //             this->createSkyboxPipeline(outColorFormat.value(), depthFormat.value(), msaaSamples.value());
    //         }
    //         else
    //         {
    //             Log::EXIT_TO_LOGGER("outColorFormat, depthFormat, and msaaSamples required for %s", this->name);
    //         }
    //     }
    //     else if (name == "post")
    //     {
    //         this->createPostProcessingPipeLine(outColorFormat.value(), depthFormat.value(), msaaSamples.value());
    //     }
    //     else if (name == "ssao")
    //     {
    //         this->createComputePipeline();
    //     }
    //     else if (name == "shadowMap")
    //     {
    //         this->createShadowMapPipeline(depthFormat.value());
    //     }
    //     else if (name == "pbrForward")
    //     {
    //         this->createForwardPBRPipeline(outColorFormat.value(), depthFormat.value(), msaaSamples.value());
    //     }
    //     else
    //     {
    //         PRINT_TO_LOGGER("Error: Unknown pipeline name: " + name);
    //     }
    // }

    // void VKPipeLineHandle::dispatch(const VkCommandBuffer &cmd, uint32_t frameIndex)
    // {
    //     assert(bindPoint_ == VK_PIPELINE_BIND_POINT_COMPUTE);

    //     submitBarriers(cmd, frameIndex);

    //     vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);

    //     bindDescriptorSets(cmd, frameIndex);

    //     // Use actual local workgroup size from shader reflection instead of hardcoded values
    //     uint32_t groupCountX = (width_ + local_size_[0] - 1) / local_size_[0];
    //     uint32_t groupCountY = (height_ + local_size_[1] - 1) / local_size_[1];
    //     vkCmdDispatch(cmd, groupCountX, groupCountY, 1);
    // }

}