#include "helperPipeline.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{
    namespace helper
    {
        namespace pipeline
        {
            VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,VkShaderModule shaderModule,cChar *entryPoint)
            {
                VkPipelineShaderStageCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                info.stage = stage;
                info.module = shaderModule;
                info.pName = entryPoint;
                return info;
            }

            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(const VkVertexInputBindingDescription &bindingDescriptions,const VkVertexInputAttributeDescription &attributeDescriptions,const cUint32_t attributeDescriptionsSize,const cUint32_t bindingDescriptionCount)
            {
                VkPipelineVertexInputStateCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                info.vertexBindingDescriptionCount = bindingDescriptionCount;
                info.vertexAttributeDescriptionCount = attributeDescriptionsSize;
                info.pVertexBindingDescriptions = &bindingDescriptions;
                info.pVertexAttributeDescriptions = &attributeDescriptions;

                return info;
            }

            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology,VkPipelineInputAssemblyStateCreateFlags flags,VkBool32 primitiveRestartEnable)
            {
                VkPipelineInputAssemblyStateCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                info.topology = topology;
                info.flags = flags;
                info.primitiveRestartEnable = primitiveRestartEnable;

                return info;
            }

            VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(const VkViewport &viewport,const VkRect2D &scissor, cUint32_t viewportCount,cUint32_t scissorCount, VkPipelineViewportStateCreateFlags flags)
            {
                VkPipelineViewportStateCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                info.viewportCount = viewportCount;
                info.pViewports = &viewport;
                info.scissorCount = scissorCount;
                info.pScissors = &scissor;
                info.flags = flags;
                return info;
            }

            VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode,VkCullModeFlags cullMode,VkFrontFace frontFace,VkBool32 depthClampEnable,VkBool32 rasterizerDiscardEnable,float lineWidth,VkBool32 depthBiasEnable,float depthBiasConstantFactor,float depthBiasClamp,float depthBiasSlopeFactor)
            {
                VkPipelineRasterizationStateCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                info.pNext = nullptr;
                info.flags = 0;
                info.depthClampEnable = depthClampEnable;
                info.rasterizerDiscardEnable = rasterizerDiscardEnable;
                info.polygonMode = polygonMode;
                info.lineWidth = lineWidth;
                info.cullMode = cullMode;
                info.frontFace = frontFace;
                info.depthBiasEnable = depthBiasEnable;
                info.depthBiasConstantFactor = depthBiasConstantFactor; // 깊이 바이어스 상수 요소를 0.0f로 설정
                info.depthBiasClamp = depthBiasClamp;                   // 깊이 바이어스 클램프를 0.0f로 설정
                info.depthBiasSlopeFactor = depthBiasSlopeFactor;       // 깊이 바이어스 슬로프 요소를 0.0f로 설정
                return info;
            }

            VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples,VkPipelineMultisampleStateCreateFlags flags)
            {
                VkPipelineMultisampleStateCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                info.rasterizationSamples = rasterizationSamples;
                info.flags = flags;
                return info;
            }

            VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable,VkBool32 depthWriteEnable,VkCompareOp depthCompareOp)
            {
                VkPipelineDepthStencilStateCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                info.depthTestEnable = depthTestEnable;
                info.depthWriteEnable = depthWriteEnable;
                info.depthCompareOp = depthCompareOp;
                info.back.compareOp = VK_COMPARE_OP_ALWAYS;

                return info;
            }

            VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(VkColorComponentFlags colorWriteMask,VkBool32 blendEnable,VkBlendOp alphaBlendOp,VkBlendOp colorBlendOp,VkBlendFactor srcColorBlendFactor,VkBlendFactor dstColorBlendFactor,VkBlendFactor srcAlphaBlendFactor,VkBlendFactor dstAlphaBlendFactor)
            {
                VkPipelineColorBlendAttachmentState info{};
                info.colorWriteMask = colorWriteMask; // 컬러 블렌딩 연산을 설정
                info.blendEnable = blendEnable;
                info.srcColorBlendFactor = srcColorBlendFactor; // 소스 컬러 블렌딩 팩터를 설정
                info.dstColorBlendFactor = dstColorBlendFactor; // 대상 컬러 블렌딩 팩터를 설정
                info.srcAlphaBlendFactor = srcAlphaBlendFactor; // 소스 알파 블렌딩 팩터를 설정
                info.dstAlphaBlendFactor = dstAlphaBlendFactor; // 대상 알파 블렌딩 팩터를 설정
                info.alphaBlendOp = alphaBlendOp;               // 알파 블렌딩 연산을 설정
                info.colorBlendOp = colorBlendOp;               // 컬러 블렌딩 연산을 설정

                return info;
            }

            // 컬러 블렌딩 상태 생성 정보 구조체
            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(cUint32_t attachmentCount,const VkPipelineColorBlendAttachmentState *pAttachments)
            {
                VkPipelineColorBlendStateCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                info.attachmentCount = attachmentCount;
                info.pAttachments = pAttachments;
                return info;
            }

            // 다이나믹 상태 설정 -> 레스터화 상태를 동적으로 변경할 수 있습니다.
            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(const std::vector<VkDynamicState> &dynamicStates)
            {
                VkPipelineDynamicStateCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                info.dynamicStateCount = static_cast<cUint32_t>(dynamicStates.size());
                info.pDynamicStates = dynamicStates.data();
                return info;
            }

            VkPushConstantRange pushConstantRange(VkShaderStageFlags stageFlags,cUint32_t size,cUint32_t offset)
            {
                VkPushConstantRange info{};
                info.stageFlags = stageFlags;
                info.offset = offset;
                info.size = size;
                return info;
            }

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const VkDescriptorSetLayout *pSetLayouts,cUint32_t setLayoutCount)
            {
                VkPipelineLayoutCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                info.setLayoutCount = setLayoutCount;
                info.pSetLayouts = pSetLayouts;
                return info;
            }

            VkGraphicsPipelineCreateInfo pipelineCreateInfo(VkPipelineLayout layout,VkRenderPass renderPass,VkPipelineCreateFlags flags)
            {
                VkGraphicsPipelineCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                info.layout = layout;
                info.renderPass = renderPass;
                info.flags = flags;
                info.basePipelineIndex = -1;
                info.basePipelineHandle = VK_NULL_HANDLE;
                return info;
            }

            VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo(std::vector<VkFormat> colorAttachmentFormats,VkFormat depthAttachmentFormat,VkFormat stencilAttachmentFormat)
            {
                VkPipelineRenderingCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
                info.colorAttachmentCount = static_cast<cUint32_t>(colorAttachmentFormats.size());
                info.pColorAttachmentFormats = colorAttachmentFormats.data();
                info.depthAttachmentFormat = depthAttachmentFormat;
                info.stencilAttachmentFormat = stencilAttachmentFormat;
                return info;
            }
        }
    }
}
