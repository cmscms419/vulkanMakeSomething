#ifndef INCLUDE_HELPER_PIPELINE_H_
#define INCLUDE_HELPER_PIPELINE_H_

#include "common.h"
#include "vkdevice.h"

namespace vkengine
{
    namespace helper
    {
        namespace pipeline
        {
            // VkPipelineShaderStageCreateInfo 생성
            // 이 함수는 셰이더 모듈과 엔트리 포인트를 사용하여 파이프라인 셰이더 스테이지 정보를 생성합니다.
            VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule, cChar *entryPoint = "main");

            // VkPipelineVertexInputStateCreateInfo 생성
            // 이 함수는 버텍스 입력 바인딩 및 속성 설명을 사용하여 파이프라인 버텍스 입력 상태 정보를 생성합니다.
            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(const VkVertexInputBindingDescription &bindingDescriptions, const VkVertexInputAttributeDescription &attributeDescriptions, const cUint32_t attributeDescriptionsSize, const cUint32_t bindingDescriptionCount);

            // VkPipelineInputAssemblyStateCreateInfo 생성
            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags, VkBool32 primitiveRestartEnable);

            // 뷰포트 설정
            VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(const VkViewport &viewport, const VkRect2D &scissor, cUint32_t viewportCount = 1, cUint32_t scissorCount = 1, VkPipelineViewportStateCreateFlags flags = 0);

            // 래스터화 설정
            VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT, VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE, VkBool32 depthClampEnable = VK_FALSE, VkBool32 rasterizerDiscardEnable = VK_FALSE, float lineWidth = 1.0f, VkBool32 depthBiasEnable = VK_FALSE, float depthBiasConstantFactor = 0.0f, float depthBiasClamp = 0.0f, float depthBiasSlopeFactor = 0.0f);

            // 다중 샘플링 설정
            VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples, VkPipelineMultisampleStateCreateFlags flags = 0);

            // 깊이 스텐실 테스트 설정
            VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp);

            VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(VkColorComponentFlags colorWriteMask, VkBool32 blendEnable, VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD, VkBlendOp colorBlendOp = VK_BLEND_OP_ADD, VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE, VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO);

            // 컬러 블렌딩 상태 생성 정보 구조체
            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(cUint32_t attachmentCount, const VkPipelineColorBlendAttachmentState *pAttachments);

            // 다이나믹 상태 설정 -> 레스터화 상태를 동적으로 변경할 수 있습니다.
            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(const std::vector<VkDynamicState> &dynamicStates);

            VkPushConstantRange pushConstantRange(VkShaderStageFlags stageFlags, cUint32_t size, cUint32_t offset);

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const VkDescriptorSetLayout *pSetLayouts, cUint32_t setLayoutCount = 1);

            VkGraphicsPipelineCreateInfo pipelineCreateInfo(VkPipelineLayout layout, VkRenderPass renderPass, VkPipelineCreateFlags flags);

            VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo(std::vector<VkFormat> colorAttachmentFormats, VkFormat depthAttachmentFormat, VkFormat stencilAttachmentFormat);

            inline VkViewport createViewport(cFloat x, cFloat y, cFloat width, cFloat height, cFloat minDepth = 0.0f, cFloat maxDepth = 1.0f)
            {
                VkViewport viewport{};
                viewport.x = x;
                viewport.y = y;
                viewport.width = width;
                viewport.height = height;
                viewport.minDepth = minDepth;
                viewport.maxDepth = maxDepth;

                return viewport;
            }

            inline VkRect2D createScissor(cInt32_t offsetX, cInt32_t offsetY, cUint32_t width, cUint32_t height)
            {
                VkRect2D scissor{};
                scissor.offset.x = offsetX;
                scissor.offset.y = offsetY;
                scissor.extent.width = width;
                scissor.extent.height = height;

                return scissor;
            }

        }
    }
}

#endif // INCLUDE_HELPER_PIPELINE_H_