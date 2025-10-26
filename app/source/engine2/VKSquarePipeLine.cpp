#include "VKpipeLineHandle.h"

using namespace vkengine::Log;

namespace vkengine {
    void VKPipeLineHandle::createSquarePipeline(
        VkFormat outColorFormat, 
        VkFormat depthFormat, 
        VkSampleCountFlagBits msaaSamples)
    {
        const cUint32_t sizeofVertexStructure = 1;
        const VkDevice device = ctx.getDevice()->logicaldevice;

        PRINT_TO_LOGGER("Creating Graphics Pipeline: " + this->name);

        // vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        // Fixed vertex input - no vertex buffers needed for procedural triangle
        VkPipelineVertexInputStateCreateInfo vertexInputCI{
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        vertexInputCI.vertexBindingDescriptionCount = 0; // No vertex buffer bindings
        vertexInputCI.pVertexBindingDescriptions = nullptr;
        vertexInputCI.vertexAttributeDescriptionCount = 0; // No vertex attributes
        vertexInputCI.pVertexAttributeDescriptions = nullptr;


        VkPipelineInputAssemblyStateCreateInfo inputAssembly = helper::pipelineInputAssemblyStateCreateInfo(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

        VkPipelineViewportStateCreateInfo viewportState = helper::pipelineViewportStateCreateInfo(
            VkViewport{},
            VkRect2D{},
            1, 1);

        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_NONE,
            VK_FRONT_FACE_COUNTER_CLOCKWISE);

        VkPipelineMultisampleStateCreateInfo multisampling =
            helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

        VkPipelineDepthStencilStateCreateInfo depthStencil =
            helper::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);

        // 0xf == VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
        //        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        VkPipelineColorBlendAttachmentState colorBlendAttachment =
            helper::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(
            1, &colorBlendAttachment);

        // dynamic states에서 뷰포트와 시저를 설정
        // 파이프라인 생성 후에 커맨드 버퍼에서 설정 가능
        VkPipelineDynamicStateCreateInfo dynamicState =
            helper::pipelineDynamicStateCreateInfo(dynamicStates);

        // 쉐이더 스테이지 생성
        // 쉐이더 모듈을 로드하고 파이프라인 쉐이더 스테이지 생성 정보를 만듭니다.
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages =
            this->shaderManager.createPipelineShaderStageCIs(this->name);

        // vulkan 1.3을 위한 Dynamic rendering 세팅을 해야한다.
        /*std::vector<VkFormat> colorFormats = { outColorFormat };
        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = helper::pipelineRenderingCreateInfo
        (
            colorFormats,
            depthFormat,
            VK_FORMAT_UNDEFINED
        );*/

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &outColorFormat;
        pipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
        
        VkGraphicsPipelineCreateInfo pipelineInfo{};

        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &pipelineRenderingCreateInfo;
        pipelineInfo.stageCount = static_cast<cUint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputCI;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = VK_NULL_HANDLE; // dynamic rendering
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        _VK_CHECK_RESULT_(
            vkCreateGraphicsPipelines(device, ctx.getPipelineCache(), 1, &pipelineInfo, nullptr, &pipeline)
        );
    }
}