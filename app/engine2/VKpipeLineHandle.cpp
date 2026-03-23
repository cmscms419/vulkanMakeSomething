#include "VKpipeLineHandle.h"

#include "log.h"
#include "type.h"
#include "helper.h"

#include <imgui.h>

using namespace vkengine::Log;

namespace vkengine
{
    VKPipeLineHandle::VKPipeLineHandle(VKcontext &context, VKShaderManager &shaderManager) : ctx(context), shaderManager(shaderManager)
    {
        pipelineLayout = VK_NULL_HANDLE;
        pipeline = VK_NULL_HANDLE;
    }

    VKPipeLineHandle::VKPipeLineHandle(VKcontext &ctx, VKShaderManager &shaderManager, cString Name, VkFormat outColorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamples) : ctx(ctx), shaderManager(shaderManager), name(Name)
    {
        pipelineLayout = VK_NULL_HANDLE;
        pipeline = VK_NULL_HANDLE;

        createByName(name, outColorFormat, depthFormat, msaaSamples);
    }

    VKPipeLineHandle::VKPipeLineHandle(VKPipeLineHandle &&other) noexcept : ctx(other.ctx), shaderManager(other.shaderManager), pipelineLayout(other.pipelineLayout), pipeline(other.pipeline), name(std::move(other.name))
    {
        other.pipelineLayout = VK_NULL_HANDLE;
        other.pipeline = VK_NULL_HANDLE;
    }

    VKPipeLineHandle::VKPipeLineHandle(VKcontext &ctx, VKShaderManager &shaderManager, const PipelineConfig &pipelineCache, std::vector<VkFormat> outColorFormats, std::optional<VkFormat> depthFormat, std::optional<VkSampleCountFlagBits> msaaSamples)
        : ctx(ctx), shaderManager(shaderManager)
    {
        pipelineLayout = VK_NULL_HANDLE;
        pipeline = VK_NULL_HANDLE;

        this->createFromConfig(pipelineCache, outColorFormats, depthFormat, msaaSamples);
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

    void VKPipeLineHandle::createFromConfig(const PipelineConfig &config, std::vector<VkFormat> outColorFormats, std::optional<VkFormat> depthFormat, std::optional<VkSampleCountFlagBits> msaaSamples)
    {
        this->name = config.name;

        // Set bindPoint_ based on pipeline type
        // this->bin = (config.type == PipelineConfig::Type::Compute) ? VK_PIPELINE_BIND_POINT_COMPUTE
        //                                                             : VK_PIPELINE_BIND_POINT_GRAPHICS;

        // Copy binding information from ShaderManager for this pipeline
        // const auto &shaderManagerResourceLayouts = this->shaderManager.getResourceLayouts()
        // auto it = shaderManagerResourceLayouts.find(this->name);
        // if (it != shaderManagerResourceLayouts.end())
        // {

        // }

        createCommon();

        if (config.type == PipelineConfig::Type::Compute)
        {
            this->createComputePipeline();
        }
        else
        {
            createGraphicsFromConfig(config, outColorFormats, depthFormat, msaaSamples);
        }
    }

    void VKPipeLineHandle::createByName(
        cString name,
        std::optional<VkFormat> outColorFormat,
        std::optional<VkFormat> depthFormat,
        std::optional<VkSampleCountFlagBits> msaaSamples)
    {
        this->name = name;

        createCommon();

        if (name == "sample_pipeline")
        {
            createSquarePipeline(outColorFormat.value(), depthFormat.value(), msaaSamples.value());
        }
        else if (name == "gui")
        {
            createGuiPipeline(outColorFormat.value());
        }
        else if (name == "sky")
        {
            if (outColorFormat.has_value() && depthFormat.has_value() && msaaSamples.has_value())
            {
                this->createSkyboxPipeline(outColorFormat.value(), depthFormat.value(), msaaSamples.value());
            }
            else
            {
                Log::EXIT_TO_LOGGER("outColorFormat, depthFormat, and msaaSamples required for %s", this->name);
            }
        }
        else if (name == "post")
        {
            this->createPostProcessingPipeLine(outColorFormat.value(), depthFormat.value(), msaaSamples.value());
        }
        else if (name == "ssao" || name == "lightdeferred")
        {
            this->createComputePipeline();
        }
        else if (name == "shadowMap")
        {
            this->createShadowMapPipeline(depthFormat.value());
        }
        else if (name == "pbrForward")
        {
            this->createForwardPBRPipeline(outColorFormat.value(), depthFormat.value(), msaaSamples.value());
        }
        else if (name == "pbrdeferred")
        {
            this->createDeferredPBRPipeline(outColorFormat.value(), depthFormat.value(), msaaSamples.value());
        }
        else
        {
            PRINT_TO_LOGGER("Error: Unknown pipeline name: " + name);
        }
    }

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

    void VKPipeLineHandle::createGraphicsFromConfig(
        const PipelineConfig &config,
        std::vector<VkFormat> outColorFormats,
        std::optional<VkFormat> depthFormat,
        std::optional<VkSampleCountFlagBits> msaaSamples)
    {
        VkDevice device = this->ctx.getDevice()->logicaldevice;

        PRINT_TO_LOGGER("Creating graphics pipeline from config: %s", config.name.c_str());

        std::vector<VkVertexInputBindingDescription> vertexInputBindings;
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

        if (config.vertexInput.type == PipelineConfig::VertexInput::Type::Standard)
        {
            // Standard 3D vertex input (PBR Forward, Shadow Map)
            vertexInputBindings.resize(1);
            vertexInputBindings[0].binding = 0;
            vertexInputBindings[0].stride = sizeof(Vertex2);
            vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            vertexInputAttributes = Vertex2::getAttributeDescriptionsAnimatedVec();
        }
        else if (config.vertexInput.type == PipelineConfig::VertexInput::Type::ImGui)
        {
            // ImGui vertex input (GUI)
            vertexInputBindings.resize(1);
            vertexInputBindings[0].binding = 0;
            vertexInputBindings[0].stride = sizeof(ImDrawVert);
            vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            vertexInputAttributes.resize(3);
            vertexInputAttributes[0].binding = 0;
            vertexInputAttributes[0].location = 0;
            vertexInputAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            vertexInputAttributes[0].offset = offsetof(ImDrawVert, ImDrawVert::pos);

            vertexInputAttributes[1].binding = 0;
            vertexInputAttributes[1].location = 1;
            vertexInputAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            vertexInputAttributes[1].offset = offsetof(ImDrawVert, ImDrawVert::uv);

            vertexInputAttributes[2].binding = 0;
            vertexInputAttributes[2].location = 2;
            vertexInputAttributes[2].format = VK_FORMAT_R8G8B8A8_UNORM;
            vertexInputAttributes[2].offset = offsetof(ImDrawVert, ImDrawVert::col);
        }

        std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCI =
            shaderManager.createPipelineShaderStageCIs(this->name);

        VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
        vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCI.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
        vertexInputStateCI.pVertexBindingDescriptions = vertexInputBindings.data();
        vertexInputStateCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInputStateCI.pVertexAttributeDescriptions = vertexInputAttributes.data();

        // ========================================================================
        // 2. INPUT ASSEMBLY STATE
        // ========================================================================
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
        inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

        // ========================================================================
        // 3. RASTERIZATION STATE
        // ========================================================================
        VkPipelineRasterizationStateCreateInfo rasterStateCI{};
        rasterStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterStateCI.depthClampEnable = config.rasterization.depthClampEnable ? VK_TRUE : VK_FALSE;
        rasterStateCI.rasterizerDiscardEnable = VK_FALSE;
        rasterStateCI.polygonMode = VK_POLYGON_MODE_FILL;
        rasterStateCI.cullMode = config.rasterization.cullMode;
        rasterStateCI.frontFace = config.rasterization.frontFace;
        rasterStateCI.depthBiasEnable = config.rasterization.depthBiasEnable ? VK_TRUE : VK_FALSE;
        rasterStateCI.depthBiasConstantFactor = config.rasterization.depthBiasConstantFactor;
        rasterStateCI.depthBiasClamp = 0.0f;
        rasterStateCI.depthBiasSlopeFactor = config.rasterization.depthBiasSlopeFactor;
        rasterStateCI.lineWidth = 1.0f;

        // ========================================================================
        // 4. COLOR BLEND STATE
        // ========================================================================
        // Create blend attachment states for each color attachment
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;

        if (!config.specialConfig.isDepthOnly && !outColorFormats.empty())
        {
            blendAttachmentStates.resize(outColorFormats.size());

            for (size_t i = 0; i < outColorFormats.size(); ++i)
            {
                auto &blendAttachmentState = blendAttachmentStates[i];
                blendAttachmentState.blendEnable = config.colorBlend.blendEnable ? VK_TRUE : VK_FALSE;
                blendAttachmentState.colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT;

                if (config.colorBlend.blendEnable)
                {
                    // Use alpha blending configuration
                    blendAttachmentState.srcColorBlendFactor =
                        config.colorBlend.alphaBlending.srcColorBlendFactor;
                    blendAttachmentState.dstColorBlendFactor =
                        config.colorBlend.alphaBlending.dstColorBlendFactor;
                    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
                    blendAttachmentState.srcAlphaBlendFactor =
                        config.colorBlend.alphaBlending.srcAlphaBlendFactor;
                    blendAttachmentState.dstAlphaBlendFactor =
                        config.colorBlend.alphaBlending.dstAlphaBlendFactor;
                    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
                }
                else
                {
                    // No blending
                    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
                    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
                }
            }
        }

        VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
        colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCI.logicOpEnable = VK_FALSE;
        colorBlendStateCI.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCI.attachmentCount = static_cast<cUint32_t>(blendAttachmentStates.size());
        colorBlendStateCI.pAttachments =
            blendAttachmentStates.empty() ? nullptr : blendAttachmentStates.data();
        colorBlendStateCI.blendConstants[0] = 0.0f;
        colorBlendStateCI.blendConstants[1] = 0.0f;
        colorBlendStateCI.blendConstants[2] = 0.0f;
        colorBlendStateCI.blendConstants[3] = 0.0f;

        // ========================================================================
        // 5. DEPTH STENCIL STATE
        // ========================================================================
        VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
        depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCI.depthTestEnable = config.depthStencil.depthTest ? VK_TRUE : VK_FALSE;
        depthStencilStateCI.depthWriteEnable = config.depthStencil.depthWrite ? VK_TRUE : VK_FALSE;
        depthStencilStateCI.depthCompareOp = config.depthStencil.depthCompareOp;
        depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCI.stencilTestEnable = VK_FALSE;
        depthStencilStateCI.front.failOp = VK_STENCIL_OP_KEEP;
        depthStencilStateCI.front.passOp = VK_STENCIL_OP_KEEP;
        depthStencilStateCI.front.depthFailOp = VK_STENCIL_OP_KEEP;
        depthStencilStateCI.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilStateCI.front.compareMask = 0;
        depthStencilStateCI.front.writeMask = 0;
        depthStencilStateCI.front.reference = 0;
        depthStencilStateCI.back = depthStencilStateCI.front;
        depthStencilStateCI.minDepthBounds = 0.0f;
        depthStencilStateCI.maxDepthBounds = 1.0f;

        // ========================================================================
        // 6. VIEWPORT STATE
        // ========================================================================
        VkPipelineViewportStateCreateInfo viewportStateCI{};
        viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCI.viewportCount = 1;
        viewportStateCI.pViewports = nullptr; // Dynamic
        viewportStateCI.scissorCount = 1;
        viewportStateCI.pScissors = nullptr; // Dynamic

        // ========================================================================
        // 7. DYNAMIC STATE
        // ========================================================================
        VkPipelineDynamicStateCreateInfo dynamicStateCI{};
        dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(config.dynamicState.states.size());
        dynamicStateCI.pDynamicStates = config.dynamicState.states.data();

        // ========================================================================
        // 8. MULTISAMPLE STATE
        // ========================================================================
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
        if (config.multisample.type == PipelineConfig::Multisample::Type::Variable &&
            msaaSamples.has_value())
        {
            sampleCount = msaaSamples.value();
        }

        VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
        multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCI.rasterizationSamples = sampleCount;
        multisampleStateCI.sampleShadingEnable = VK_FALSE;
        multisampleStateCI.minSampleShading = 1.0f;
        multisampleStateCI.pSampleMask = nullptr;
        multisampleStateCI.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCI.alphaToOneEnable = VK_FALSE;

        // ========================================================================
        // 9. PIPELINE RENDERING CREATE INFO (Vulkan 1.3 Dynamic Rendering)
        // ========================================================================
        VkPipelineRenderingCreateInfo pipelineRenderingCI{};
        pipelineRenderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCI.viewMask = 0;

        // For depth-only pipelines (shadow maps), don't set any color attachments
        if (config.specialConfig.isDepthOnly)
        {
            pipelineRenderingCI.colorAttachmentCount = 0;
            pipelineRenderingCI.pColorAttachmentFormats = nullptr;
        }
        else
        {
            pipelineRenderingCI.colorAttachmentCount = static_cast<uint32_t>(outColorFormats.size());
            pipelineRenderingCI.pColorAttachmentFormats =
                outColorFormats.empty() ? nullptr : outColorFormats.data();
        }

        // Set depth format
        if (depthFormat.has_value())
        {
            pipelineRenderingCI.depthAttachmentFormat = depthFormat.value();

            // Only set stencil format if the depth format has a stencil aspect
            // Common depth-stencil formats: VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT
            // Depth-only formats: VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT
            switch (depthFormat.value())
            {
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
                // These formats have stencil component
                pipelineRenderingCI.stencilAttachmentFormat = depthFormat.value();
                break;
            default:
                // Depth-only formats don't have stencil
                pipelineRenderingCI.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
                break;
            }
        }
        else
        {
            pipelineRenderingCI.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
            pipelineRenderingCI.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
        }

        // ========================================================================
        // 10. GRAPHICS PIPELINE CREATE INFO
        // ========================================================================
        VkGraphicsPipelineCreateInfo pipelineCI{};
        pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCI.pNext = &pipelineRenderingCI;
        pipelineCI.flags = 0;
        pipelineCI.stageCount = static_cast<uint32_t>(shaderStagesCI.size());
        pipelineCI.pStages = shaderStagesCI.data();
        pipelineCI.pVertexInputState = &vertexInputStateCI;
        pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
        pipelineCI.pTessellationState = nullptr;
        pipelineCI.pViewportState = &viewportStateCI;
        pipelineCI.pRasterizationState = &rasterStateCI;
        pipelineCI.pMultisampleState = &multisampleStateCI;
        pipelineCI.pDepthStencilState = &depthStencilStateCI;
        pipelineCI.pColorBlendState = &colorBlendStateCI;
        pipelineCI.pDynamicState = &dynamicStateCI;
        pipelineCI.layout = this->getPipelineLayout();
        pipelineCI.renderPass = VK_NULL_HANDLE; // Using dynamic rendering
        pipelineCI.subpass = 0;
        pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCI.basePipelineIndex = -1;

        // Create the graphics pipeline
        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(device, ctx.getPipelineCache(), 1, &pipelineCI, nullptr, &pipeline));

        PRINT_TO_LOGGER("Successfully created graphics pipeline: %s", config.name.c_str());
    }

}