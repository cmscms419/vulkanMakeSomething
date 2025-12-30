#include "pipeLineHandle.h"
#include <imgui.h>

using namespace vkengine::Log;

namespace vkengine {

    // Vertex/Index는 수동으로 설정
    // Reflection으로는 자료형이 애매하기 때문
    // (셰이더에서 vertex/index 자료형을 지정하지 않음)
    // 개수 확인 정도는 가능
    void PipeLineHandle::createGuiPipeline(VkFormat outColorFormat)
    {
#if 0
        // vertex attribute description
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(ImDrawVert);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(ImDrawVert, ImDrawVert::pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(ImDrawVert, ImDrawVert::uv);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[2].offset = offsetof(ImDrawVert, ImDrawVert::col);

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages =
            shaderManager.createPipelineShaderStageCIs(this->name);

        std::vector<VkFormat> colorFormats(outColorFormat);

        VkPipelineVertexInputStateCreateInfo vertexInputCI =
            helper::pipelineVertexInputStateCreateInfo(
                bindingDescription,
                *attributeDescriptions.data(),
                static_cast<cUint32_t>(attributeDescriptions.size()),
                static_cast<cUint32_t>(1)
            );

        VkPipelineInputAssemblyStateCreateInfo inputAssembly =
            helper::pipelineInputAssemblyStateCreateInfo(
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                VK_FALSE,
                VK_FALSE
            );

        VkPipelineRasterizationStateCreateInfo rasterStateCI;
        rasterStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterStateCI.pNext = nullptr;
        rasterStateCI.flags = 0;
        rasterStateCI.depthClampEnable = VK_FALSE;
        rasterStateCI.rasterizerDiscardEnable = VK_FALSE;
        rasterStateCI.polygonMode = VK_POLYGON_MODE_FILL;
        rasterStateCI.cullMode = VK_CULL_MODE_NONE;
        rasterStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterStateCI.depthBiasEnable = VK_FALSE;
        rasterStateCI.depthBiasConstantFactor = 0.0f;
        rasterStateCI.depthBiasClamp = 0.0f;
        rasterStateCI.depthBiasSlopeFactor = 0.0f;
        rasterStateCI.lineWidth = 1.0f;

        VkPipelineColorBlendAttachmentState blendAttachmentState =
            helper::pipelineColorBlendAttachmentState(
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
                VK_TRUE,
                VK_BLEND_OP_ADD,
                VK_BLEND_OP_ADD,
                VK_BLEND_FACTOR_SRC_ALPHA,
                VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                VK_BLEND_FACTOR_ZERO
            );

        VkPipelineColorBlendStateCreateInfo colorBlendCI =
            helper::pipelineColorBlendStateCreateInfo(
                1,
                &blendAttachmentState
            );

        colorBlendCI.logicOpEnable = VK_FALSE;
        colorBlendCI.logicOp = VK_LOGIC_OP_COPY;
        colorBlendCI.blendConstants[0] = 0.0f;
        colorBlendCI.blendConstants[1] = 0.0f;
        colorBlendCI.blendConstants[2] = 0.0f;
        colorBlendCI.blendConstants[3] = 0.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
        depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCI.depthTestEnable = VK_FALSE;
        depthStencilStateCI.depthWriteEnable = VK_FALSE;
        depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCI.stencilTestEnable = VK_FALSE;
        depthStencilStateCI.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilStateCI.minDepthBounds = 0.0f;
        depthStencilStateCI.maxDepthBounds = 1.0f;

        VkPipelineViewportStateCreateInfo viewportStateCI;
        viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCI.pNext = nullptr;
        viewportStateCI.flags = 0;
        viewportStateCI.viewportCount = 1;
        viewportStateCI.pViewports = nullptr; // Dynamic
        viewportStateCI.scissorCount = 1;
        viewportStateCI.pScissors = nullptr; // Dynamic

        VkPipelineDynamicStateCreateInfo dynamicStateCI =
            helper::pipelineDynamicStateCreateInfo(
                dynamicStates
            );

        VkPipelineMultisampleStateCreateInfo multisampling =
            helper::pipelineMultisampleStateCreateInfo(
                VK_SAMPLE_COUNT_1_BIT
            );

        VkPipelineRenderingCreateInfo pipelineRenderingCI =
            helper::pipelineRenderingCreateInfo(
                colorFormats,
                VK_FORMAT_UNDEFINED,
                VK_FORMAT_UNDEFINED
            );

        VkGraphicsPipelineCreateInfo pipeline{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        pipeline.pNext = &pipelineRenderingCI;
        pipeline.flags = 0;
        pipeline.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipeline.pStages = shaderStages.data();
        pipeline.pVertexInputState = &vertexInputCI;
        pipeline.pInputAssemblyState = &inputAssembly;
        pipeline.pTessellationState = nullptr;
        pipeline.pViewportState = &viewportStateCI;
        pipeline.pRasterizationState = &rasterStateCI;
        pipeline.pMultisampleState = &multisampling;
        pipeline.pDepthStencilState = &depthStencilStateCI;
        pipeline.pColorBlendState = &colorBlendCI;
        pipeline.pDynamicState = &dynamicStateCI;
        pipeline.layout = pipelineLayout;
        pipeline.renderPass = VK_NULL_HANDLE;
        pipeline.subpass = 0;
        pipeline.basePipelineHandle = VK_NULL_HANDLE;
        pipeline.basePipelineIndex = -1;
#else
        // vertex attribute description
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(ImDrawVert);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(ImDrawVert, ImDrawVert::pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(ImDrawVert, ImDrawVert::uv);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[2].offset = offsetof(ImDrawVert, ImDrawVert::col);

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages =
            shaderManager.createPipelineShaderStageCIs(this->name);

        std::vector<VkFormat> colorFormats = { outColorFormat };

        VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
        vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCI.vertexBindingDescriptionCount =
            static_cast<uint32_t>(1);
        vertexInputStateCI.pVertexBindingDescriptions = &bindingDescription;
        vertexInputStateCI.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputStateCI.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI;
        inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCI.pNext = nullptr;
        inputAssemblyStateCI.flags = 0;
        inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

        VkPipelineRasterizationStateCreateInfo rasterStateCI;
        rasterStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterStateCI.pNext = nullptr;
        rasterStateCI.flags = 0;
        rasterStateCI.depthClampEnable = VK_FALSE;
        rasterStateCI.rasterizerDiscardEnable = VK_FALSE;
        rasterStateCI.polygonMode = VK_POLYGON_MODE_FILL;
        rasterStateCI.cullMode = VK_CULL_MODE_NONE;
        rasterStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterStateCI.depthBiasEnable = VK_FALSE;
        rasterStateCI.depthBiasConstantFactor = 0.0f;
        rasterStateCI.depthBiasClamp = 0.0f;
        rasterStateCI.depthBiasSlopeFactor = 0.0f;
        rasterStateCI.lineWidth = 1.0f;

        VkPipelineColorBlendAttachmentState blendAttachmentState{};
        blendAttachmentState.blendEnable = VK_TRUE;
        blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCI;
        colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCI.pNext = nullptr;
        colorBlendStateCI.flags = 0;
        colorBlendStateCI.logicOpEnable = VK_FALSE;
        colorBlendStateCI.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCI.attachmentCount = 1;
        colorBlendStateCI.pAttachments = &blendAttachmentState;
        colorBlendStateCI.blendConstants[0] = 0.0f;
        colorBlendStateCI.blendConstants[1] = 0.0f;
        colorBlendStateCI.blendConstants[2] = 0.0f;
        colorBlendStateCI.blendConstants[3] = 0.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
        depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCI.depthTestEnable = VK_FALSE;
        depthStencilStateCI.depthWriteEnable = VK_FALSE;
        depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCI.stencilTestEnable = VK_FALSE;
        depthStencilStateCI.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilStateCI.minDepthBounds = 0.0f;
        depthStencilStateCI.maxDepthBounds = 1.0f;
        // depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

        VkPipelineViewportStateCreateInfo viewportStateCI;
        viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCI.pNext = nullptr;
        viewportStateCI.flags = 0;
        viewportStateCI.viewportCount = 1;
        viewportStateCI.pViewports = nullptr; // Dynamic
        viewportStateCI.scissorCount = 1;
        viewportStateCI.pScissors = nullptr; // Dynamic

        std::vector<VkDynamicState> dynamicStateEnables_;
        dynamicStateEnables_ = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        VkPipelineDynamicStateCreateInfo dynamicStateCI;
        dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCI.pNext = nullptr;
        dynamicStateCI.flags = 0;
        dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables_.size());
        dynamicStateCI.pDynamicStates = dynamicStateEnables_.data();

        VkPipelineMultisampleStateCreateInfo multisampleStateCI;
        multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCI.pNext = nullptr;
        multisampleStateCI.flags = 0;
        multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCI.sampleShadingEnable = VK_FALSE;
        multisampleStateCI.minSampleShading = 1.0f;
        multisampleStateCI.pSampleMask = nullptr;
        multisampleStateCI.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCI.alphaToOneEnable = VK_FALSE;

        VkPipelineRenderingCreateInfo pipelineRenderingCI;
        pipelineRenderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCI.pNext = nullptr;
        pipelineRenderingCI.viewMask = 0;
        pipelineRenderingCI.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
        pipelineRenderingCI.pColorAttachmentFormats = colorFormats.data();
        pipelineRenderingCI.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipelineRenderingCI.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        VkGraphicsPipelineCreateInfo pipeline;
        pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline.pNext = &pipelineRenderingCI;
        pipeline.flags = 0;
        pipeline.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipeline.pStages = shaderStages.data();
        pipeline.pVertexInputState = &vertexInputStateCI;
        pipeline.pInputAssemblyState = &inputAssemblyStateCI;
        pipeline.pTessellationState = nullptr;
        pipeline.pViewportState = &viewportStateCI;
        pipeline.pRasterizationState = &rasterStateCI;
        pipeline.pMultisampleState = &multisampleStateCI;
        pipeline.pDepthStencilState = &depthStencilStateCI;
        pipeline.pColorBlendState = &colorBlendStateCI;
        pipeline.pDynamicState = &dynamicStateCI;
        pipeline.layout = pipelineLayout;
        pipeline.renderPass = VK_NULL_HANDLE;
        pipeline.subpass = 0;
        pipeline.basePipelineHandle = VK_NULL_HANDLE;
        pipeline.basePipelineIndex = -1;
#endif
        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(
            this->ctx.getDevice()->logicaldevice,
            this->ctx.getPipelineCache(),
            1,
            &pipeline,
            nullptr,
            &this->pipeline
        ));

    }
}