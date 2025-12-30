#include "VKrenderer.h"


using namespace vkengine::Log;

namespace vkengine {
    
    VKforwardRenderer::VKforwardRenderer(VKcontext& ctx, VKShaderManager& shadermanager, const cUint32_t& MaxFramesFlight, const cString& assetsPath, const cString& shaderPath)
        : 
        ctx(ctx), shaderManager(shadermanager), MaxFramesFlight(MaxFramesFlight),
        assetsPath(assetsPath), shaderPath(shaderPath),
        dummyTexture(ctx), msaaColorBuffer(ctx), depthStencil(ctx), msaaDepthStencil(ctx),
        skyTextures(ctx), shadowMap(ctx), samplerLinearRepeat(ctx), samplerLinearClamp(ctx),
        samplerAnisoRepeat(ctx), samplerAnisoClamp(ctx), forwardToCompute(ctx), computeToPost(ctx)
    {
    }
    
    VKforwardRenderer::~VKforwardRenderer()
    {
        this->cleanup();
    }
    
    void VKforwardRenderer::prepareForModels(std::vector<VKModel>& models, VkFormat outColorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamples, uint32_t swapChainWidth, uint32_t swapChainHeight)
    {
        createPipelines(outColorFormat, depthFormat, msaaSamples);
        createTextures(swapChainWidth, swapChainHeight, msaaSamples);
        createUniformBuffers();

        for (VKModel& model : models)
        {
            model.createDescriptorManager2(samplerLinearRepeat, dummyTexture);
        }

    }
    
    void VKforwardRenderer::createPipelines(const VkFormat colorFormat, const VkFormat depthFormat, VkSampleCountFlagBits msaaSamples)
    {
        pipelines.emplace("pbrForward",
            PipeLineHandle(ctx, shaderManager, "pbrForward", VK_FORMAT_R16G16B16A16_SFLOAT,
                depthFormat, msaaSamples));
        pipelines.emplace("sky", PipeLineHandle(ctx, shaderManager, "sky", VK_FORMAT_R16G16B16A16_SFLOAT,
            depthFormat, msaaSamples));
        pipelines.emplace("post", PipeLineHandle(ctx, shaderManager, "post", colorFormat,
            depthFormat, VK_SAMPLE_COUNT_1_BIT));
        pipelines.emplace("shadowMap", PipeLineHandle(ctx, shaderManager, "shadowMap", VK_FORMAT_D16_UNORM,
            VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT));
    }
    
    void VKforwardRenderer::createTextures(uint32_t swapchainWidth, uint32_t swapchainHeight, VkSampleCountFlagBits msaaSamples)
    {
        this->samplerLinearRepeat.createLinearRepeat();
        this->samplerLinearClamp.createLinearClamp();
        this->samplerAnisoRepeat.createAnisoRepeat();
        this->samplerAnisoClamp.createAnisoClamp();

        cString dummyImagePath = this->assetsPath + "CustomUVChecker_byValle_2K.png";

        cUint32_t width, height;

        cUChar* pixels = load_png_rgba(dummyImagePath.c_str(), &width, &height, TextureType::Texture_rgb_alpha);

        if (!pixels) {
            EXIT_TO_LOGGER("Failed to load texture image : %s", dummyImagePath.c_str());
        }

        this->dummyTexture.createTextureFromPixelData(pixels, width, height, TextureType::Texture_rgb_alpha, true);

        if (pixels != nullptr && dummyImagePath.find(".png") != std::string::npos) {
            free(pixels);
        }

        this->dummyTexture.setSampler(this->samplerLinearRepeat.getSampler());

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
        skyDescriptorSet.create(ctx, { this->skyTextures.Prefiltered().ResourceBinding(),
                                        this->skyTextures.Irradiance().ResourceBinding(),
                                        this->skyTextures.BrdfLUT().ResourceBinding() });

        // Create descriptor set for shadow mapping
        shadowMapSet.create(ctx, { this->shadowMap.getResouceBinding()});
    }

    void VKforwardRenderer::createUniformBuffers() {
        const VkDevice device = ctx.getDevice()->logicaldevice;

        // Create scene uniform buffers
        this->sceneDataUniform.clear();
        this->sceneDataUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i) {
            sceneDataUniform.emplace_back(this->ctx, sceneDataUBO);
        }

        // Create options uniform buffers
        optionsUniform.clear();
        optionsUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i) {
            optionsUniform.emplace_back(this->ctx, optionsUBO);
        }

        skyOptionsUniform.clear();
        skyOptionsUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i) {
            skyOptionsUniform.emplace_back(this->ctx, skyOptionsUBO);
        }

        postOptionsUniform.clear();
        postOptionsUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i) {
            postOptionsUniform.emplace_back(this->ctx, postOptionsUBO);
        }

        boneDataUniform.clear();
        boneDataUniform.reserve(this->MaxFramesFlight);
        for (uint32_t i = 0; i < this->MaxFramesFlight; ++i) {
            boneDataUniform.emplace_back(this->ctx, boneDataUBO);
        }

        SceneSkyOptionsStates.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++) {
            SceneSkyOptionsStates[i].create(
                this->ctx, { sceneDataUniform[i].ResourceBinding(), skyOptionsUniform[i].ResourceBinding()});
        }

        PostDescriptorSets.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++) {
            PostDescriptorSets[i].create(
                this->ctx, { forwardToCompute.ResourceBinding(), postOptionsUniform[i].ResourceBinding() });
        }

        SceneOptionsBoneDataSets.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++) {
            SceneOptionsBoneDataSets[i].create(this->ctx, { 
                                                       sceneDataUniform[i].ResourceBinding(),
                                                       optionsUniform[i].ResourceBinding(),
                                                       boneDataUniform[i].ResourceBinding() });
        }
    }

    void VKforwardRenderer::update(object::Camera2& camera, uint32_t currentFrame, double time)
    {
        this->sceneDataUniform[currentFrame].updateData();
        this->optionsUniform[currentFrame].updateData();
        this->skyOptionsUniform[currentFrame].updateData();
        this->postOptionsUniform[currentFrame].updateData();
    }
    
    void VKforwardRenderer::updateBoneData(const std::vector<VKModel>& models, uint32_t currentFrame)
    {
        // Reset bone data
        boneDataUBO.animationData.x = 0.0f;
        for (int i = 0; i < 256; ++i) {
            boneDataUBO.boneMatrices[i] = glm::mat4(1.0f);
        }

        // Check if any model has animation data
        bool hasAnyAnimation = false;
        for (const auto& model : models) {
            if (model.hasAnimations() && model.hasBones()) {
                hasAnyAnimation = true;

                // Get bone matrices from the first animated model
                const auto& boneMatrices = model.getBoneMatrices();

                // Copy bone matrices (up to 256 bones)
                size_t bonesToCopy = std::min(boneMatrices.size(), static_cast<size_t>(256));
                for (size_t i = 0; i < bonesToCopy; ++i) {
                    boneDataUBO.boneMatrices[i] = boneMatrices[i];
                }

                break; // For now, use the first animated model
            }
        }

        boneDataUBO.animationData.x = float(hasAnyAnimation);

        // DEBUG: Log hasAnimation state
        static bool lastHasAnimation = false;
        if (lastHasAnimation != hasAnyAnimation) {
            PRINT_TO_LOGGER("hasAnimation changed to: &s", hasAnyAnimation ? "true" : "false");
            lastHasAnimation = hasAnyAnimation;
        }

        // Update the GPU buffer
        boneDataUniform[currentFrame].updateData();
    }
    
    void VKforwardRenderer::draw(VkCommandBuffer cmd, uint32_t currentFrame, VkImageView swapchainImageView, std::vector<VKModel>& models, VkViewport viewport, VkRect2D scissor)
    {
        VkRect2D renderArea = { 0, 0, scissor.extent.width, scissor.extent.height };

        // Forward rendering pass
        {
            forwardToCompute.transitionTo(
                cmd,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

            auto colorAttachment = createColorAttachment(
                msaaColorBuffer.getImageView(), VK_ATTACHMENT_LOAD_OP_CLEAR, {0.0f, 0.0f, 0.5f, 0.0f},
                forwardToCompute.getImageView(), VK_RESOLVE_MODE_AVERAGE_BIT);

            auto depthAttachment =
                createDepthAttachment(
                    msaaDepthStencil.view, VK_ATTACHMENT_LOAD_OP_CLEAR, 1.0f,
                    depthStencil.view, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
            
            auto renderingInfo = createRenderingInfo(renderArea, &colorAttachment, &depthAttachment);

            vkCmdBeginRendering(cmd, &renderingInfo);
            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            VkDeviceSize offsets[1]{ 0 };

            // Render models
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelines.at("pbrForward").getPipeline());

            for (size_t j = 0; j < models.size(); j++) {
                if (!models[j].Visible()) {
                    continue;
                }

                vkCmdPushConstants(cmd, pipelines.at("pbrForward").getPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                    sizeof(models[j].ModelMatrix()), &models[j].ModelMatrix());
                vkCmdPushConstants(cmd, pipelines.at("pbrForward").getPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    sizeof(models[j].ModelMatrix()), sizeof(float) * 16,
                    models[j].Coeffs());

                for (size_t i = 0; i < models[j].Meshes().size(); i++) {

                    auto& mesh = models[j].Meshes()[i];

                    // Skip culled meshes
                    if (mesh.isCulled) {
                        continue;
                    }

                    uint32_t matIndex = mesh.materialIndex;

                    const auto descriptorSets =
                        std::vector{ 
                               this->SceneOptionsBoneDataSets[currentFrame].get(),
                               models[j].MaterialDescriptorSetsManager(matIndex).get(),
                               skyDescriptorSet.get(),
                               shadowMapSet.get()
                        };

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

            const auto skyDescriptorSets = std::vector
            {
                SceneSkyOptionsStates[currentFrame].get(), // Set 0: scene + sky options
                skyDescriptorSet.get()                   // Set 1: sky textures
            };

            vkCmdBindDescriptorSets(
                cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.at("sky").getPipelineLayout(), 0,
                static_cast<uint32_t>(skyDescriptorSets.size()), skyDescriptorSets.data(), 0, nullptr);
            vkCmdDraw(cmd, 36, 1, 0, 0);
            vkCmdEndRendering(cmd);
        }

        // Post-processing pass
        {
            forwardToCompute.transitionTo(
                cmd, 
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_ACCESS_2_SHADER_READ_BIT, 
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

            auto colorAttachment = createColorAttachment(
                swapchainImageView, VK_ATTACHMENT_LOAD_OP_CLEAR, { 0.0f, 0.0f, 1.0f, 0.0f });

            // No depth attachment needed for post-processing
            auto renderingInfo = createRenderingInfo(renderArea, &colorAttachment, nullptr);

            vkCmdBeginRendering(cmd, &renderingInfo);
            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetScissor(cmd, 0, 1, &scissor);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines.at("post").getPipeline());

            const auto postDescriptorSets =
                std::vector{ this->PostDescriptorSets[currentFrame].get()};
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelines.at("post").getPipelineLayout(), 0,
                static_cast<uint32_t>(postDescriptorSets.size()),
                postDescriptorSets.data(), 0, nullptr);

            vkCmdDraw(cmd, 6, 1, 0, 0);
            vkCmdEndRendering(cmd);
        }
    }
    
    void VKforwardRenderer::makeShadowMap(VkCommandBuffer cmd, uint32_t currentFrame, std::vector<VKModel>& models)
    {
        // ±íÀÌ ¸Ê »ý¼ºÀ» À§ÇÑ ½¦µµ¿ì ÆÐ½º(±íÀÌ ¸ÊÀ» »ý¼ºÇÏ´Â ·»´õ¸µ ÆÐ½º)
        VkImageMemoryBarrier2 shadowMapBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        shadowMapBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        shadowMapBarrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        shadowMapBarrier.srcAccessMask = VK_ACCESS_2_NONE;
        shadowMapBarrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        shadowMapBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        shadowMapBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowMapBarrier.image = this->shadowMap.getImage();
        shadowMapBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
        shadowMapBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadowMapBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        VkDependencyInfo depInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &shadowMapBarrier;
        vkCmdPipelineBarrier2(cmd, &depInfo);

        // ±×¸²ÀÚ ¸Ê ·»´õ¸µ ½ÃÀÛ
        VkRenderingAttachmentInfo shadowDepthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        shadowDepthAttachment.imageView = this->shadowMap.getImageView();
        shadowDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        shadowDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        shadowDepthAttachment.clearValue.depthStencil = { 1.0f, 0 };

        VkRenderingInfo shadowRenderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
        shadowRenderingInfo.renderArea = { 0, 0, this->shadowMap.getWidth(), this->shadowMap.getHeight() };
        shadowRenderingInfo.layerCount = 1;
        shadowRenderingInfo.colorAttachmentCount = 0;
        shadowRenderingInfo.pDepthAttachment = &shadowDepthAttachment;

        VkViewport shadowViewport{ 0.0f, 0.0f, (float)this->shadowMap.getWidth(), (float)this->shadowMap.getHeight(),
                                  0.0f, 1.0f };
        VkRect2D shadowScissor{ 0, 0, this->shadowMap.getWidth(), this->shadowMap.getHeight() };

        vkCmdBeginRendering(cmd, &shadowRenderingInfo);
        vkCmdSetViewport(cmd, 0, 1, &shadowViewport);
        vkCmdSetScissor(cmd, 0, 1, &shadowScissor);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines.at("shadowMap").getPipeline());

        const auto descriptorSets = std::vector{ this->SceneOptionsBoneDataSets[currentFrame].get()};

        vkCmdBindDescriptorSets(
            cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines.at("shadowMap").getPipelineLayout(), 0,
            static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

        vkCmdSetDepthBias(cmd,
            1.1f,  // Constant factor
            0.0f,  // Clamp value
            2.0f); // Slope factor

        // Render all visible models to shadow map
        VkDeviceSize offsets[1]{ 0 };

        for (size_t j = 0; j < models.size(); j++) {
            if (!models[j].Visible()) {
                continue;
            }

            vkCmdPushConstants(cmd, this->pipelines.at("shadowMap").getPipelineLayout(),
                VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(models[j].ModelMatrix()),
                &models[j].ModelMatrix());

            // Render all meshes in this model
            for (size_t i = 0; i < models[j].Meshes().size(); i++) {
                auto& mesh = models[j].Meshes()[i];

                // Skip culled meshes in shadow pass too
                if (mesh.isCulled) {
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

        // Transition shadow map to shader read-only for sampling in main render pass
        VkImageMemoryBarrier2 shadowMapReadBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        shadowMapReadBarrier.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        shadowMapReadBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        shadowMapReadBarrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        shadowMapReadBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        shadowMapReadBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowMapReadBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadowMapReadBarrier.image = this->shadowMap.getImage();
        shadowMapReadBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
        shadowMapReadBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadowMapReadBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        VkDependencyInfo readDepInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        readDepInfo.imageMemoryBarrierCount = 1;
        readDepInfo.pImageMemoryBarriers = &shadowMapReadBarrier;
        vkCmdPipelineBarrier2(cmd, &readDepInfo);
    }
    
    auto VKforwardRenderer::getCullingStats() const -> const CullingStats&
    {
        return this->cullingStats;
    }
    bool VKforwardRenderer::isFrustumCullingEnabled() const
    {
        return this->frustumCullingEnabled;
    }
    
    void VKforwardRenderer::performFrustumCulling(std::vector<VKModel>& models)
    {
        cullingStats.totalMeshes = 0;
        cullingStats.culledMeshes = 0;
        cullingStats.renderedMeshes = 0;

        if (!frustumCullingEnabled) {
            for (auto& model : models) {
                for (auto& mesh : model.Meshes()) {
                    mesh.isCulled = false;
                    cullingStats.totalMeshes++;
                    cullingStats.renderedMeshes++;
                }
            }
            return;
        }

        for (auto& model : models) {
            for (auto& mesh : model.Meshes()) {
                cullingStats.totalMeshes++;

                bool isVisible = viewFrustum.intersects(mesh.worldBounds);

                mesh.isCulled = !isVisible;

                if (isVisible) {
                    cullingStats.renderedMeshes++;
                }
                else {
                    cullingStats.culledMeshes++;
                }
            }
        }
    }
    
    void VKforwardRenderer::setFrustumCullingEnabled(bool enabled)
    {
        this->frustumCullingEnabled = enabled;
    }
    void VKforwardRenderer::updateViewFrustum(const cMat4& viewProjection)
    {
        if (this->frustumCullingEnabled) {
            this->viewFrustum.extractFromViewProjection(viewProjection);
        }
    }

    VkRenderingAttachmentInfo VKforwardRenderer::createColorAttachment(VkImageView imageView, VkAttachmentLoadOp loadOp, VkClearColorValue clearColor, VkImageView resolveImageView, VkResolveModeFlagBits resolveMode) const
    {
        VkRenderingAttachmentInfo attachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
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

    VkRenderingAttachmentInfo VKforwardRenderer::createDepthAttachment(VkImageView imageView, VkAttachmentLoadOp loadOp, float clearDepth, VkImageView resolveImageView, VkResolveModeFlagBits resolveMode) const
    {
        VkRenderingAttachmentInfo attachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        attachment.imageView = imageView;
        attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment.loadOp = loadOp;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.clearValue.depthStencil = { clearDepth, 0 };
        attachment.resolveMode = resolveMode;
        attachment.resolveImageView = resolveImageView;
        attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        return attachment;
    }

    VkRenderingInfo VKforwardRenderer::createRenderingInfo(const VkRect2D& renderArea, const VkRenderingAttachmentInfo* colorAttachment, const VkRenderingAttachmentInfo* depthAttachment) const
    {
        VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
        renderingInfo.renderArea = renderArea;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = colorAttachment ? 1 : 0;
        renderingInfo.pColorAttachments = colorAttachment;
        renderingInfo.pDepthAttachment = depthAttachment;
        renderingInfo.pStencilAttachment = depthAttachment;
        return renderingInfo;
    }
}