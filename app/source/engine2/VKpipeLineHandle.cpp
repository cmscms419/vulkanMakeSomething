#include "VKpipeLineHandle.h"

using namespace vkengine::Log;

namespace vkengine {

    void VKPipeLineHandle::cleanup()

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

    void VKPipeLineHandle::createCommon()
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

    void VKPipeLineHandle::createGraphicsPipeline(
        VkFormat outColorFormat,
        VkFormat depthFormat,
        VkSampleCountFlagBits msaaSamples)
    {
        // 임시로 사각형 파이프라인 생성 함수 호출
        this->createSquarePipeline(
            outColorFormat,
            depthFormat,
            msaaSamples);
    }

}