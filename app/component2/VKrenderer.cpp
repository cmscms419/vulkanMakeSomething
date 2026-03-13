#include "VKrenderer.h"

#include "log.h"
#include "resourseload.h"

using namespace vkengine::Log;

namespace vkengine
{

    VKRenderer::VKRenderer(
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
        PRINT_TO_LOGGER("Renderer2 created with MaxFramesFlight: %d assetsPath: %s shaderPath: %s",
                        MaxFramesFlight,
                        assetsPath.c_str(),
                        shaderPath.c_str());
    }

    VKRenderer::~VKRenderer()
    {
        this->cleanup();
    }

    void VKRenderer::cleanup()
    {
    }
    void VKRenderer::update(object::Camera2 &camera, uint32_t currentFrame, double time)
    {
        this->sceneDataUniform[currentFrame].updateData();
        this->optionsUniform[currentFrame].updateData();
        this->skyOptionsUniform[currentFrame].updateData();
        this->postOptionsUniform[currentFrame].updateData();
    }
    void VKRenderer::rendering(VkCommandBuffer cmd, uint32_t currentFrame, uint32_t imageIndex, std::vector<VKModel> &models, VkViewport viewport, VkRect2D scissor)
    {
        this->currentModels = &models;
        this->currentScissor = scissor;
        this->currentViewport = viewport;

        this->renderGraph.execute(cmd, currentFrame, imageIndex);

        this->currentModels = nullptr;
    }

    void VKRenderer::buildRenderGraph(VKSwapChain &swapchain)
    {
        this->renderGraph.registerSwapchainResource("swapchain", swapchain);      // swapchain 리소스 등록
        this->renderGraph.registerResource("shadowDepth", shadowMap);             // 쉐도우 맵 리소스 등록
        this->renderGraph.registerResource("forwardToCompute", forwardToCompute); // 포워드 패스 출력 등록
        this->renderGraph.registerResource("computeToPost", computeToPost);       // 컴퓨트 패스 출력 등록

        RenderPassNode shadowPass{
            "shadow",
            {},
            {{"shadowDepth", ResourceAccess::DepthAttachmentWrite}},
            [this](VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageIndex)
            {
                this->makeShadowMap(cmd, frameIndex, imageIndex);
            }};
        RenderPassNode forwardPass{
            "pbrForward",
            {{"shadowDepth", ResourceAccess::ShaderReadOnly}},
            {{"forwardToCompute", ResourceAccess::ColorAttachmentWrite}},
            [this](VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageIndex)
            {
                this->makeForwardPBRPass(cmd, frameIndex, imageIndex);
            }};

        RenderPassNode postProcessPass{
            "postProcess",
            {{"forwardToCompute", ResourceAccess::ShaderReadOnly}},
            {{"swapchain", ResourceAccess::Present}},
            [this](VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageIndex)
            {
                this->makePostProcessPass(cmd, frameIndex, imageIndex);
            }};

        this->renderGraph.addPass(shadowPass);
        this->renderGraph.addPass(forwardPass);
        this->renderGraph.addPass(postProcessPass);

        this->renderGraph.compile();
    }

    void VKRenderer::prepareForModels(std::vector<VKModel> &models, VkFormat outColorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamples, cUint32_t swapChainWidth, cUint32_t swapChainHeight)
    {
        this->createPipelines(outColorFormat, depthFormat, msaaSamples);
        this->createTextures(swapChainWidth, swapChainHeight, msaaSamples);
        this->createUniformBuffers();

        for (VKModel &model : models)
        {
            model.createDescriptorManager2(samplerLinearRepeat, dummyTexture);
        }
    }

    void VKRenderer::createPipelines(const VkFormat colorFormat, const VkFormat depthFormat, VkSampleCountFlagBits msaaSamples)
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

    void VKRenderer::createTextures(cUint32_t swapchainWidth, cUint32_t swapchainHeight, VkSampleCountFlagBits msaaSamples)
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
        this->msaaDepthStencil.createDepthStencil(swapchainWidth, swapchainHeight, msaaSamples);
        this->depthStencil.createDepthStencil(swapchainWidth, swapchainHeight, VK_SAMPLE_COUNT_1_BIT);
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

    void VKRenderer::resize(uint32_t width, uint32_t height, VkSampleCountFlagBits msaaSamples)
    {
        // 크기에 의존하는 이미지 정리
        this->msaaColorBuffer.cleanup();
        this->msaaDepthStencil.cleanup();
        this->depthStencil.cleanup();
        this->forwardToCompute.cleanup();
        this->computeToPost.cleanup();

        // 새 크기로 재생성
        this->msaaColorBuffer.createMsaaColorBuffer(width, height, msaaSamples);
        this->msaaDepthStencil.createDepthStencil(width, height, msaaSamples);
        this->depthStencil.createDepthStencil(width, height, VK_SAMPLE_COUNT_1_BIT);
        this->forwardToCompute.createGeneralStorage(width, height);
        this->computeToPost.createGeneralStorage(width, height);

        this->forwardToCompute.setSampler(this->samplerLinearRepeat.getSampler());

        // PostDescriptorSets는 forwardToCompute를 참조하므로 재생성
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            PostDescriptorSets[i].create(
                this->ctx, {std::ref(forwardToCompute), std::ref(postOptionsUniform[i].Buffer())});
        }
    }

    void VKRenderer::createUniformBuffers()
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

        PostDescriptorSets.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            PostDescriptorSets[i].create(
                this->ctx, {std::ref(forwardToCompute), std::ref(postOptionsUniform[i].Buffer())});
        }

        SceneOptionsBoneDataSets.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            SceneOptionsBoneDataSets[i].create(this->ctx, {std::ref(sceneDataUniform[i].Buffer()),
                                                           std::ref(optionsUniform[i].Buffer()),
                                                           std::ref(boneDataUniform[i].Buffer())});
        }
    }

    void VKRenderer::makeForwardPBRPass(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
        VkRect2D renderArea = {0, 0, this->currentScissor.extent.width, this->currentScissor.extent.height};

        auto colorAttachment = createColorAttachment(
            msaaColorBuffer.getImageView(), VK_ATTACHMENT_LOAD_OP_CLEAR, {0.0f, 0.0f, 0.5f, 0.0f},
            forwardToCompute.getImageView(), VK_RESOLVE_MODE_AVERAGE_BIT);

        auto depthAttachment =
            createDepthAttachment(
                msaaDepthStencil.getImageView(), VK_ATTACHMENT_LOAD_OP_CLEAR, 1.0f,
                depthStencil.getImageView(), VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);

        auto renderingInfo = createRenderingInfo(renderArea, &colorAttachment, &depthAttachment);

        vkCmdBeginRendering(cmd, &renderingInfo);
        vkCmdSetViewport(cmd, 0, 1, &this->currentViewport);
        vkCmdSetScissor(cmd, 0, 1, &this->currentScissor);

        VkDeviceSize offsets[1]{0};

        // Render models
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipelines.at("pbrForward").getPipeline());

        for (size_t j = 0; j < this->currentModels->size(); j++)
        {
            if (!this->currentModels->at(j).Visible())
            {
                continue;
            }

            vkCmdPushConstants(cmd, pipelines.at("pbrForward").getPipelineLayout(),
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(this->currentModels->at(j).ModelResource()), &this->currentModels->at(j).ModelResource());

            for (size_t i = 0; i < this->currentModels->at(j).Meshes().size(); i++)
            {

                auto &mesh = this->currentModels->at(j).Meshes()[i];

                // Skip culled meshes
                if (mesh.isCulled)
                {
                    continue;
                }

                uint32_t matIndex = mesh.materialIndex;

                const auto descriptorSets =
                    std::vector{
                        this->SceneOptionsBoneDataSets[currentFrame].get(),
                        this->currentModels->at(j).MaterialDescriptorSetsManager(matIndex).get(),
                        skyDescriptorSet.get(),
                        shadowMapSet.get()};

                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipelines.at("pbrForward").getPipelineLayout(), 0,
                                        static_cast<uint32_t>(descriptorSets.size()),
                                        descriptorSets.data(), 0, nullptr);

                vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex->Buffer(), offsets);
                vkCmdBindIndexBuffer(cmd, mesh.index->Buffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
            }
        }

        // Sky rendering pass
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at("sky").getPipeline());

        const auto skyDescriptorSets = std::vector{
            SceneSkyOptionsStates[currentFrame].get(), // Set 0: scene + sky options
            skyDescriptorSet.get()                     // Set 1: sky textures
        };

        vkCmdBindDescriptorSets(
            cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at("sky").getPipelineLayout(), 0,
            static_cast<uint32_t>(skyDescriptorSets.size()), skyDescriptorSets.data(), 0, nullptr);
        vkCmdDraw(cmd, 36, 1, 0, 0);
        vkCmdEndRendering(cmd);
    }

    void VKRenderer::makePostProcessPass(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
        VkRect2D renderArea = {0, 0, this->currentScissor.extent.width, this->currentScissor.extent.height};
        VkImageView &swapchainImageView = this->renderGraph.getVKSwapChain().getSwapChainImageView(imageIndex);

        if (swapchainImageView == VK_NULL_HANDLE)
        {
            EXIT_TO_LOGGER("swapchainImageView us null");
        }

        auto colorAttachment = createColorAttachment(
            swapchainImageView, VK_ATTACHMENT_LOAD_OP_CLEAR, {0.0f, 0.0f, 1.0f, 0.0f});

        // No depth attachment needed for post-processing
        auto renderingInfo = createRenderingInfo(renderArea, &colorAttachment, nullptr);

        vkCmdBeginRendering(cmd, &renderingInfo);
        vkCmdSetViewport(cmd, 0, 1, &this->currentViewport);
        vkCmdSetScissor(cmd, 0, 1, &this->currentScissor);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines.at("post").getPipeline());

        const auto postDescriptorSets =
            std::vector{this->PostDescriptorSets[currentFrame].get()};
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelines.at("post").getPipelineLayout(), 0,
                                static_cast<uint32_t>(postDescriptorSets.size()),
                                postDescriptorSets.data(), 0, nullptr);

        vkCmdDraw(cmd, 6, 1, 0, 0);
        vkCmdEndRendering(cmd);
    }

    void VKRenderer::makeShadowMap(VkCommandBuffer cmd, uint32_t currentFrame, cUint32_t imageIndex)
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

        // Render all visible this->currentModels to shadow map
        VkDeviceSize offsets[1]{0};

        for (size_t j = 0; j < this->currentModels->size(); j++)
        {
            if (!this->currentModels->at(j).Visible())
            {
                continue;
            }

            vkCmdPushConstants(cmd, this->pipelines.at("shadowMap").getPipelineLayout(),
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(this->currentModels->at(j).ModelResource().modelMatrix),
                               &this->currentModels->at(j).ModelResource().modelMatrix);

            // Render all meshes in this model
            for (size_t i = 0; i < this->currentModels->at(j).Meshes().size(); i++)
            {
                auto &mesh = this->currentModels->at(j).Meshes()[i];

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

    VkRenderingAttachmentInfo VKRenderer::createColorAttachment(VkImageView imageView, VkAttachmentLoadOp loadOp, VkClearColorValue clearColor, VkImageView resolveImageView, VkResolveModeFlagBits resolveMode) const
    {
        VkRenderingAttachmentInfo attachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
        attachment.imageView = imageView;
        attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.loadOp = loadOp;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.clearValue.color = clearColor;
        attachment.resolveMode = resolveMode;
        attachment.resolveImageView = resolveImageView;
        attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        return attachment;
    }

    VkRenderingAttachmentInfo VKRenderer::createDepthAttachment(VkImageView imageView, VkAttachmentLoadOp loadOp, float clearDepth, VkImageView resolveImageView, VkResolveModeFlagBits resolveMode) const
    {
        VkRenderingAttachmentInfo attachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
        attachment.imageView = imageView;
        attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment.loadOp = loadOp;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.clearValue.depthStencil = {clearDepth, 0};
        attachment.resolveMode = resolveMode;
        attachment.resolveImageView = resolveImageView;
        attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        return attachment;
    }

    VkRenderingInfo VKRenderer::createRenderingInfo(const VkRect2D &renderArea, const VkRenderingAttachmentInfo *colorAttachment, const VkRenderingAttachmentInfo *depthAttachment) const
    {
        VkRenderingInfo renderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
        renderingInfo.renderArea = renderArea;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = colorAttachment ? 1 : 0;
        renderingInfo.pColorAttachments = colorAttachment;
        renderingInfo.pDepthAttachment = depthAttachment;
        renderingInfo.pStencilAttachment = depthAttachment;
        return renderingInfo;
    }

    const CullingStats &VKRenderer::getCullingStats() const
    {
        return this->cullingStats;
    }

    cBool VKRenderer::isFrustumCullingEnabled() const
    {
        return this->frustumCullingEnabled;
    }

    void VKRenderer::performFrustumCulling(std::vector<VKModel> &models)
    {
        cullingStats.totalMeshes = 0;
        cullingStats.culledMeshes = 0;
        cullingStats.renderedMeshes = 0;

        if (!frustumCullingEnabled)
        {
            for (auto &model : models)
            {
                for (auto &mesh : model.Meshes())
                {
                    mesh.isCulled = false;
                    cullingStats.totalMeshes++;
                    cullingStats.renderedMeshes++;
                }
            }
            return;
        }

        for (auto &model : models)
        {
            for (auto &mesh : model.Meshes())
            {
                cullingStats.totalMeshes++;

                bool isVisible = viewFrustum.intersects(mesh.worldBounds);
                mesh.isCulled = !isVisible;

                if (isVisible)
                    cullingStats.renderedMeshes++;
                else
                    cullingStats.culledMeshes++;
            }
        }
    }

    void VKRenderer::setFrustumCullingEnabled(bool enabled)
    {
        this->frustumCullingEnabled = enabled;
    }

    void VKRenderer::updateViewFrustum(const cMat4 &viewProjection)
    {
        if (this->frustumCullingEnabled)
        {
            this->viewFrustum.extractFromViewProjection(viewProjection);
        }
    }

}