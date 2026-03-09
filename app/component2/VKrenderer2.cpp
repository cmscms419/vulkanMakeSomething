#include "VKrenderer2.h"

#include "log.h"
#include "resourseload.h"

using namespace vkengine::Log;

namespace vkengine
{

    VKforwardRenderer2::VKforwardRenderer2(
        VKcontext &ctx,
        VKShaderManager &shadermanager,
        const cUint32_t &MaxFramesFlight,
        const cString &assetsPath,
        const cString &shaderPath)
        : ctx(ctx), renderGraph(ctx), shaderManager(shadermanager),
          MaxFramesFlight(MaxFramesFlight), assetsPath(assetsPath), shaderPath(shaderPath),
          dummyTexture(ctx), msaaColorBuffer(ctx), depthStencil(ctx), msaaDepthStencil(ctx),
          skyTextures(ctx), shadowMap(ctx), samplerLinearRepeat(ctx), samplerLinearClamp(ctx),
          samplerAnisoRepeat(ctx), samplerAnisoClamp(ctx), forwardToCompute(ctx), computeToPost(ctx),
          samplerShadowMap(ctx)
    {
        PRINT_TO_LOGGER("VKforwardRenderer2 created with MaxFramesFlight: %d assetsPath: %s shaderPath: %s",
                        MaxFramesFlight,
                        assetsPath.c_str(),
                        shaderPath.c_str());
        this->currentModels = nullptr;
    }

    VKforwardRenderer2::~VKforwardRenderer2()
    {
        this->cleanup();
    }

    void VKforwardRenderer2::cleanup()
    {
    }

    void VKforwardRenderer2::buildRenderGraph(VKSwapChain &swapchain)
    {
        this->renderGraph.registerSwapchainResource("swapchain", swapchain); // swapchain 리소스 등록
        this->renderGraph.registerResource("shadowDepth", shadowMap);   // 쉐도우 맵 리소스 등록

        this->renderGraph.registerResource("prefilteredMap", skyTextures.Prefiltered()); // 스카이박스 리소스 등록
        this->renderGraph.registerResource("irradianceMap", skyTextures.Irradiance());   // 스카이박스 리소스 등록
        this->renderGraph.registerResource("brdfLUT", skyTextures.BrdfLUT());            // 스카이박스 리소스 등록

        this->renderGraph.registerResource("forwardToCompute", forwardToCompute); // 포워드 패스 출력 등록

        RenderPassNode shadowPass{
            "shadow",
            {},
            {
                // No outputs to other passes, but we will read this in the forward pass
                {"shadowDepth", ResourceAccess::DepthAttachmentWrite},
            },
            [this](VkCommandBuffer cmd, cUint32_t frameIndex)
            {
                this->executeShadowPass(cmd, frameIndex);
            }};
        RenderPassNode forwardPass{
            "forward",
            {
                {"shadowDepth", ResourceAccess::ShaderReadOnly},
            },
            {
                {"forwardToCompute", ResourceAccess::ShaderReadWrite},
            },
            [this](VkCommandBuffer cmd, cUint32_t frameIndex)
            {
                this->executeForwardPass(cmd, frameIndex);
            }};
        RenderPassNode PresentPass{
            "post",
            {
                {"forwardToCompute", ResourceAccess::ShaderReadOnly},
            },
            {
                {"swapchain", ResourceAccess::Present},
            },
            [this](VkCommandBuffer cmd, cUint32_t frameIndex)
            {
                this->executePostPass(cmd, frameIndex);
            }};

        this->renderGraph.addPass(shadowPass);
        this->renderGraph.addPass(forwardPass);

        this->renderGraph.compile();
    }

    void VKforwardRenderer2::prepareForModels(std::vector<VKModel> &models, VkFormat outColorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamples, cUint32_t swapChainWidth, cUint32_t swapChainHeight)
    {
        this->createPipelines(outColorFormat, depthFormat, msaaSamples);
        this->createTextures(swapChainWidth, swapChainHeight, msaaSamples);
        this->createUniformBuffers();

        for (VKModel &model : models)
        {
            model.createDescriptorManager2(samplerLinearRepeat, dummyTexture);
        }

        this->currentModels = &models;
    }

    void VKforwardRenderer2::executeShadowPass(VkCommandBuffer cmd, cUint32_t frameIndex)
    {
        if (this->shadowMap.getImage() == VK_NULL_HANDLE)
        {
            return; // Shadow map not initialized
        }

        if (this->currentModels->empty())
        {
            return; // No currentModels set for shadow pass
        }

        this->makeShadowMap(cmd, frameIndex);
    }

    void VKforwardRenderer2::executeForwardPass(VkCommandBuffer cmd, cUint32_t frameIndex)
    {
    }

    void VKforwardRenderer2::executePostPass(VkCommandBuffer cmd, cUint32_t frameIndex)
    {
    }

    void VKforwardRenderer2::createPipelines(const VkFormat colorFormat, const VkFormat depthFormat, VkSampleCountFlagBits msaaSamples)
    {
        pipelines.emplace("pbrForward",
                          VKPipeLineHandle(ctx, shaderManager, "pbrForward", VK_FORMAT_R16G16B16A16_SFLOAT,
                                           depthFormat, msaaSamples));
        pipelines.emplace("sky", VKPipeLineHandle(ctx, shaderManager, "sky", VK_FORMAT_R16G16B16A16_SFLOAT,
                                                  depthFormat, msaaSamples));
        pipelines.emplace("post", VKPipeLineHandle(ctx, shaderManager, "post", colorFormat,
                                                   depthFormat, VK_SAMPLE_COUNT_1_BIT));
        pipelines.emplace("shadowMap", VKPipeLineHandle(ctx, shaderManager, "shadowMap", VK_FORMAT_D16_UNORM,
                                                        VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT));
    }

    void VKforwardRenderer2::createTextures(cUint32_t swapchainWidth, cUint32_t swapchainHeight, VkSampleCountFlagBits msaaSamples)
    {
        this->samplerLinearRepeat.createLinearRepeat();
        this->samplerLinearClamp.createLinearClamp();
        this->samplerAnisoRepeat.createAnisoRepeat();
        this->samplerAnisoClamp.createAnisoClamp();
        this->samplerShadowMap.createShadowMapSampler();

        cString dummyImagePath = this->assetsPath + "CustomUVChecker_byValle_2K.png";
        cUint32_t width, height;
        cUChar *pixels = load_png_rgba(dummyImagePath.c_str(), &width, &height, TextureType::Texture_rgb_alpha);

        if (!pixels)
        {
            EXIT_TO_LOGGER("Failed to load texture image : %s", dummyImagePath.c_str());
        }

        this->dummyTexture.createTextureFromPixelData(pixels, width, height, TextureType::Texture_rgb_alpha, true);

        if (pixels != nullptr && dummyImagePath.find(".png") != std::string::npos)
        {
            free(pixels);
        }

        this->dummyTexture.setSampler(this->samplerLinearRepeat.getSampler());

        // Initialize shadow map texture
        this->shadowMap.createShadowMap(2048 * 2, 2048 * 2);
        this->shadowMap.setSampler(this->samplerShadowMap.getSampler());

        // Initialize IBL textures for PBR
        cString path = this->assetsPath + "cubeMap/";
        this->skyTextures.LoadKTXMap(
            path + "specular_out.ktx2",
            path + "diffuse_out.ktx2",
            path + "outputLUT.png");

        // Create render targets
        this->msaaColorBuffer.createMsaaColorBuffer(swapchainWidth, swapchainHeight, msaaSamples);
        this->msaaDepthStencil.create(swapchainWidth, swapchainHeight, msaaSamples);
        this->depthStencil.create(swapchainWidth, swapchainHeight, VK_SAMPLE_COUNT_1_BIT);
        this->forwardToCompute.createGeneralStorage(swapchainWidth, swapchainHeight);
        this->computeToPost.createGeneralStorage(swapchainWidth, swapchainHeight);

        // Set samplers
        forwardToCompute.setSampler(samplerLinearRepeat.getSampler());

        // Create descriptor sets for sky textures (set 1 for sky pipeline)
        skyDescriptorSet.create(ctx, {std::ref(this->skyTextures.Prefiltered()),
                                      std::ref(this->skyTextures.Irradiance()),
                                      std::ref(this->skyTextures.BrdfLUT())});

        // Create descriptor set for shadow mapping
        shadowMapSet.create(ctx, {std::ref(this->shadowMap)});
    }

    void VKforwardRenderer2::createUniformBuffers()
    {
        const VkDevice device = ctx.getDevice()->logicaldevice;

        // Create scene uniform buffers
        this->sceneDataUniform.clear();
        this->sceneDataUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i)
        {
            sceneDataUniform.emplace_back(this->ctx, sceneDataUBO);
        }

        // Create options uniform buffers
        optionsUniform.clear();
        optionsUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i)
        {
            optionsUniform.emplace_back(this->ctx, optionsUBO);
        }

        skyOptionsUniform.clear();
        skyOptionsUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i)
        {
            skyOptionsUniform.emplace_back(this->ctx, skyOptionsUBO);
        }

        postOptionsUniform.clear();
        postOptionsUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i)
        {
            postOptionsUniform.emplace_back(this->ctx, postOptionsUBO);
        }

        boneDataUniform.clear();
        boneDataUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i)
        {
            boneDataUniform.emplace_back(this->ctx, boneDataUBO);
        }

        SceneSkyOptionsStates.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            SceneSkyOptionsStates[i].create(
                this->ctx, {std::ref(sceneDataUniform[i].Buffer()), std::ref(skyOptionsUniform[i].Buffer())});
        }

        // PostDescriptorSets.resize(this->MaxFramesFlight);
        // for (size_t i = 0; i < this->MaxFramesFlight; i++)
        // {
        //     PostDescriptorSets[i].create(
        //         this->ctx, {std::ref(forwardToCompute), std::ref(postOptionsUniform[i].Buffer())});
        // }

        SceneOptionsBoneDataSets.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            SceneOptionsBoneDataSets[i].create(this->ctx, {std::ref(sceneDataUniform[i].Buffer()),
                                                           std::ref(optionsUniform[i].Buffer()),
                                                           std::ref(boneDataUniform[i].Buffer())});
        }
    }

    void VKforwardRenderer2::makeShadowMap(VkCommandBuffer cmd, uint32_t currentFrame)
    {
#if 1
        // 그림자 맵 렌더링 시작
        VkRenderingAttachmentInfo shadowDepthAttachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
        shadowDepthAttachment.imageView = this->shadowMap.getImageView();
        shadowDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        shadowDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        shadowDepthAttachment.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo shadowRenderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
        shadowRenderingInfo.renderArea = {0, 0, this->shadowMap.getWidth(), this->shadowMap.getHeight()};
        shadowRenderingInfo.layerCount = 1;
        shadowRenderingInfo.colorAttachmentCount = 0;
        shadowRenderingInfo.pDepthAttachment = &shadowDepthAttachment;

        VkViewport shadowViewport{0.0f, 0.0f, (float)this->shadowMap.getWidth(), (float)this->shadowMap.getHeight(),
                                  0.0f, 1.0f};
        VkRect2D shadowScissor{0, 0, this->shadowMap.getWidth(), this->shadowMap.getHeight()};

        vkCmdBeginRendering(cmd, &shadowRenderingInfo);
        vkCmdSetViewport(cmd, 0, 1, &shadowViewport);
        vkCmdSetScissor(cmd, 0, 1, &shadowScissor);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines.at("shadowMap").getPipeline());

        const auto descriptorSets = std::vector{this->SceneOptionsBoneDataSets[currentFrame].get()};

        vkCmdBindDescriptorSets(
            cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines.at("shadowMap").getPipelineLayout(), 0,
            static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

        vkCmdSetDepthBias(cmd,
                          1.1f,  // Constant factor
                          0.0f,  // Clamp value
                          2.0f); // Slope factor

        // Render all visible currentModels to shadow map
        VkDeviceSize offsets[1]{0};

        for (size_t j = 0; j < currentModels->size(); j++)
        {
            if (!currentModels->at(j).Visible())
            {
                continue;
            }

            vkCmdPushConstants(cmd, this->pipelines.at("shadowMap").getPipelineLayout(),
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(currentModels->at(j).ModelMatrix()),
                               &currentModels->at(j).ModelMatrix());

            // Render all meshes in this model
            for (size_t i = 0; i < currentModels->at(j).Meshes().size(); i++)
            {
                auto &mesh = currentModels->at(j).Meshes()[i];

                // Skip culled meshes in shadow pass too
                if (mesh.isCulled)
                {
                    continue;
                }

                // Bind vertex and index buffers
                vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex->Buffer(), offsets);
                vkCmdBindIndexBuffer(cmd, mesh.index->Buffer(), 0, VK_INDEX_TYPE_UINT32);

                // Draw the mesh
                vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
            }
        }

        vkCmdEndRendering(cmd);
#else

#endif
    }

}