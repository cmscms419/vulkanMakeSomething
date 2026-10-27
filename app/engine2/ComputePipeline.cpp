#include "PipelineHandle.h"
#include "log.h"


namespace vkengine
{
    void PipelineHandle::createComputePipeline()
    {
        Log::printToLogger("Creating compute pipeline: %s", this->name.c_str());

        const VkDevice device = this->ctx.getDevice()->logicaldevice;
        const VkPipelineCache pipelineCache = this->ctx.getPipelineCache();

        // Get shader stage create info for the compute shader
        std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCI = 
            this->shaderManager.makePipelineShaderStageCIs(this->name);

        if (shaderStagesCI.empty())
        {
            Log::exitToLogger("No compute shader stages found for pipeline: %s", this->name.c_str());
        }

        if (shaderStagesCI.size() != 1)
        {
            Log::exitToLogger("Compute pipeline must have exactly one shader stage, found: %zu", shaderStagesCI.size());
        }

        if (shaderStagesCI[0].stage != VK_SHADER_STAGE_COMPUTE_BIT)
        {
            Log::exitToLogger("Expected compute shader stage, but got different stage type");
        }

        // Create compute pipeline
        VkComputePipelineCreateInfo pipelineCI{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        pipelineCI.layout = this->pipelineLayout;
        pipelineCI.stage = shaderStagesCI[0]; // Only one shader stage for compute
        pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCI.basePipelineIndex = -1;

        _VK_CHECK_RESULT_(vkCreateComputePipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &this->pipeline));

        Log::printToLogger("Successfully created compute pipeline");
    }
}