#include "VKrenderer.h"

#include "log.h"
#include "vkconfig.h"
#include "resourseload.h"
#include "helper.h"

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
          samplerAnisoRepeat(ctx), samplerAnisoClamp(ctx), DeferredToCompute(ctx), LightDeferred(ctx),
          samplerShadowMap(ctx), materialStorageBuffer(ctx), table(ctx)
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
    void VKRenderer::update(object::Camera2 &camera, cUint32_t currentFrame, double time)
    {
        this->sceneDataUniform[currentFrame].updateData();
        this->optionsUniform[currentFrame].updateData();
        this->skyOptionsUniform[currentFrame].updateData();
        this->postOptionsUniform[currentFrame].updateData();
        this->ssaoParamsUniform[currentFrame].updateData();
    }
    void VKRenderer::rendering(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex, std::vector<VKModel> &models, VkViewport viewport, VkRect2D scissor)
    {
        this->currentModels = &models;
        this->currentScissor = scissor;
        this->currentViewport = viewport;

        this->renderGraph.execute(cmd, currentFrame, imageIndex);

        this->currentModels = nullptr;
    }

    void VKRenderer::buildRenderGraph(VKSwapChain &swapchain)
    {
        this->renderGraph.registerSwapchainResource("swapchain", swapchain);        // swapchain 리소스 등록
        this->renderGraph.registerResource("shadowDepth", shadowMap);               // 쉐도우 맵 리소스 등록
        this->renderGraph.registerResource("DeferredToCompute", DeferredToCompute); // 포워드 패스 출력 등록
        this->renderGraph.registerResource("LightDeferred", LightDeferred);         // 컴퓨트 패스 출력 등록
        this->renderGraph.registerResource("depthStencil", depthStencil);           // depthstencil 리소스 등록

        this->renderGraph.loadFromJson(this->assetsPath + "/renderGraph.json");

        this->renderGraph.registerPassFunction("shadow",
                                               [this](VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageIndex)
                                               {
                                                   this->makeShadowMap(cmd, frameIndex, imageIndex);
                                               });

        this->renderGraph.registerPassFunction("pbrdeferred",
                                               [this](VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageIndex)
                                               {
                                                   this->makePBRDeferredPass(cmd, frameIndex, imageIndex);
                                               });

        this->renderGraph.registerPassFunction("lightdeferred",
                                               [this](VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageIndex)
                                               {
                                                   this->makeLightDeferredPass(cmd, frameIndex, imageIndex);
                                               });

        this->renderGraph.registerPassFunction("postProcess",
                                               [this](VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageIndex)
                                               {
                                                   this->makePostProcessPass(cmd, frameIndex, imageIndex);
                                               });

        this->renderGraph.compile();
    }

    void VKRenderer::prepareForModels(std::vector<VKModel> &models, VkFormat outColorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamples, cUint32_t swapChainWidth, cUint32_t swapChainHeight)
    {
        this->createPipelines(outColorFormat, depthFormat, msaaSamples);
        this->createTextures(swapChainWidth, swapChainHeight, msaaSamples);
        this->createUniformBuffers();

        std::vector<cMaterial> allMaterials;

        for (VKModel &model : models)
        {
            model.createDescriptorManager2(samplerLinearRepeat, allMaterials, table);
        }

        VkDeviceSize size = sizeof(cMaterial) * allMaterials.size();

        materialStorageBuffer.createStorageBuffer(size,
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        materialStorageBuffer.copyData(allMaterials.data(), size);
        materialDescriptorSet.create(ctx, {std::ref(this->materialStorageBuffer),
                                           std::ref(this->table)});
    }

    void VKRenderer::createPipelines(const VkFormat colorFormat, const VkFormat depthFormat, VkSampleCountFlagBits msaaSamples)
    {
        VkFormat selectedHDRFormat = selectOptimalHDRFormat(false, true); // No alpha, moderate precision
        pipelines.emplace("pbrdeferred",
                          VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createPbrDeferred(),
                                           std::vector<VkFormat>{selectedHDRFormat, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                 VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM},
                                           depthFormat, msaaSamples));
        pipelines.emplace("sky", VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createSky(), std::vector<VkFormat>{selectedHDRFormat},
                                                  depthFormat, msaaSamples));
        pipelines.emplace("post", VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createPost(), std::vector<VkFormat>{colorFormat},
                                                   depthFormat, VK_SAMPLE_COUNT_1_BIT));
        pipelines.emplace("shadowMap", VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createShadowMap(), std::vector<VkFormat>{},
                                                        VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT));
        pipelines.emplace("lightdeferred", VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createDeferredLighting(), std::vector<VkFormat>{},
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
        this->depthStencil.createDepthStencil(swapchainWidth, swapchainHeight, VK_SAMPLE_COUNT_1_BIT, true);
        this->DeferredToCompute.createGeneralStorage(swapchainWidth, swapchainHeight);
        this->LightDeferred.createGeneralStorage(swapchainWidth, swapchainHeight);

        // Set samplers
        DeferredToCompute.setSampler(samplerLinearRepeat.getSampler());
        depthStencil.setSampler(samplerLinearRepeat.getSampler());
        LightDeferred.setSampler(samplerLinearRepeat.getSampler());

        // Create descriptor sets for sky textures (set 1 for sky pipeline)
        skyDescriptorSet.create(ctx, {std::ref(this->skyTextures.Prefiltered()),
                                      std::ref(this->skyTextures.Irradiance()),
                                      std::ref(this->skyTextures.BrdfLUT())});

        // Create descriptor set for shadow mapping
        shadowMapSet.create(ctx, {std::ref(this->shadowMap)});

        // Initialize image buffers (simplified - no MSAA)
        const std::vector<cString> imageNames = {"gAlbedo", "gNormal", "gPosition", "gMaterial"};

        for (const auto &name : imageNames)
        {
            this->images[name] = std::make_unique<VKImage2D>(ctx);
        }

        // Create G-buffer textures for deferred rendering
        PRINT_TO_LOGGER("Creating G-buffer textures for deferred rendering:");

        // G-Buffer format selection for optimal memory usage and precision
        VkFormat albedoFormat = VK_FORMAT_R8G8B8A8_UNORM;        // Albedo + Metallic (4 bytes)
        VkFormat normalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;   // Normal + Roughness (8 bytes, needs precision)
        VkFormat positionFormat = VK_FORMAT_R32G32B32A32_SFLOAT; // Position + Depth (16 bytes, needs high precision)
        VkFormat materialFormat = VK_FORMAT_R8G8B8A8_UNORM;      // AO + Emissive + Material ID (4 bytes)

        // G-buffer usage flags (similar to floatColor but without storage bit since they're render targets)
        VkImageUsageFlags gBufferUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                         VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VkDevice logicaldevice = ctx.getDevice()->logicaldevice;
        VkPhysicalDevice physicalDevice = ctx.getDevice()->physicalDevice;

        // Create gAlbedo buffer (Albedo RGB + Metallic A)
        // images["gAlbedo"]->createImage(
        images["gAlbedo"]->createImage(
            swapchainWidth,
            swapchainHeight,
            albedoFormat,
            VK_SAMPLE_COUNT_1_BIT,
            gBufferUsage,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);
        images["gAlbedo"]->setSampler(samplerLinearRepeat.getSampler());

        // Create gNormal buffer (World Normal RGB + Roughness A)
        images["gNormal"]->createImage(
            swapchainWidth,
            swapchainHeight,
            normalFormat,
            VK_SAMPLE_COUNT_1_BIT,
            gBufferUsage,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);
        images["gNormal"]->setSampler(samplerLinearRepeat.getSampler());

        // Create gPosition buffer (World Position RGB + Depth A)
        images["gPosition"]->createImage(
            swapchainWidth,
            swapchainHeight,
            positionFormat,
            VK_SAMPLE_COUNT_1_BIT,
            gBufferUsage,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);
        images["gPosition"]->setSampler(samplerLinearRepeat.getSampler());

        // Create gMaterial buffer (AO R + Emissive Intensity G + Material ID B + Unused A)
        images["gMaterial"]->createImage(
            swapchainWidth,
            swapchainHeight,
            materialFormat,
            VK_SAMPLE_COUNT_1_BIT,
            gBufferUsage,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);
        images["gMaterial"]->setSampler(samplerLinearRepeat.getSampler());

        PRINT_TO_LOGGER("G-buffer creation complete");
    }

    void VKRenderer::resize(cUint32_t width, cUint32_t height, VkSampleCountFlagBits msaaSamples)
    {
        // 크기에 의존하는 이미지 정리
        this->msaaColorBuffer.cleanup();
        this->msaaDepthStencil.cleanup();
        this->depthStencil.cleanup();
        this->DeferredToCompute.cleanup();
        this->LightDeferred.cleanup();

        // 새 크기로 재생성
        this->msaaColorBuffer.createMsaaColorBuffer(width, height, msaaSamples);
        this->msaaDepthStencil.createDepthStencil(width, height, msaaSamples);
        this->depthStencil.createDepthStencil(width, height, VK_SAMPLE_COUNT_1_BIT);
        this->DeferredToCompute.createGeneralStorage(width, height);
        this->LightDeferred.createGeneralStorage(width, height);

        this->DeferredToCompute.setSampler(this->samplerLinearRepeat.getSampler());

        // PostDescriptorSets는 forwardToCompute를 참조하므로 재생성
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            PostDescriptorSets[i].create(
                this->ctx, {std::ref(DeferredToCompute),
                            std::ref(postOptionsUniform[i].Buffer())});
        }
    }

    void VKRenderer::createUniformBuffers()
    {
        const VkDevice device = ctx.getDevice()->logicaldevice;

        // Create scene uniform buffers
        this->sceneDataUniform.clear();
        this->sceneDataUniform.reserve(this->MaxFramesFlight);
        for (cUint32_t i = 0; i < this->MaxFramesFlight; ++i)
        {
            sceneDataUniform.emplace_back(this->ctx, sceneDataUBO);
        }

        // Create options uniform buffers
        optionsUniform.clear();
        optionsUniform.reserve(this->MaxFramesFlight);
        for (cUint32_t i = 0; i < this->MaxFramesFlight; ++i)
        {
            optionsUniform.emplace_back(this->ctx, optionsUBO);
        }

        skyOptionsUniform.clear();
        skyOptionsUniform.reserve(this->MaxFramesFlight);
        for (cUint32_t i = 0; i < this->MaxFramesFlight; ++i)
        {
            skyOptionsUniform.emplace_back(this->ctx, skyOptionsUBO);
        }

        postOptionsUniform.clear();
        postOptionsUniform.reserve(this->MaxFramesFlight);
        for (cUint32_t i = 0; i < this->MaxFramesFlight; ++i)
        {
            postOptionsUniform.emplace_back(this->ctx, postOptionsUBO);
        }

        ssaoParamsUniform.clear();
        ssaoParamsUniform.reserve(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            ssaoParamsUniform.emplace_back(this->ctx, ssaoParamsUBO);
        }

        boneDataUniform.clear();
        boneDataUniform.reserve(this->MaxFramesFlight);
        for (cUint32_t i = 0; i < this->MaxFramesFlight; ++i)
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
                this->ctx, {std::ref(LightDeferred), std::ref(postOptionsUniform[i].Buffer())});
        }

        SceneOptionsBoneDataSets.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            SceneOptionsBoneDataSets[i].create(this->ctx, {std::ref(sceneDataUniform[i].Buffer()),
                                                           std::ref(optionsUniform[i].Buffer()),
                                                           std::ref(boneDataUniform[i].Buffer())});
        }

        // SSAO 디스크립터 세트 생성
        // forwardToCompute 및 computeToPost가 스토리지 바인딩에 맞게 올바르게 구성되었는지 확인
        // forwardToCompute는 읽기 전용 스토리지 이미지(입력)로 사용됩니다.
        // computeToPost는 쓰기 전용 스토리지 이미지(출력)로 사용됩니다.
        // 이미지 레이아웃 전환을 위한 명령 버퍼 생성
        // set을 만들기 위해서 변환
        VKCommandBufferHander cmd = ctx.createGrapicsCommandBufferHander(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        this->DeferredToCompute.transitionTo(
            cmd.getCommandBuffer(),
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_2_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        this->LightDeferred.transitionTo(
            cmd.getCommandBuffer(),
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_2_SHADER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        cmd.submitAndWait();
        ssaoDescriptorSets.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            ssaoDescriptorSets[i].create(this->ctx, {
                                                        std::ref(sceneDataUniform[i].Buffer()),
                                                        std::ref(ssaoParamsUniform[i].Buffer()),
                                                        std::ref(this->DeferredToCompute),
                                                        std::ref(this->LightDeferred),
                                                        std::ref(this->depthStencil),
                                                    });
        }
    }

    void VKRenderer::makeForwardPBRPass(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
        VkRect2D renderArea = {0, 0, this->currentScissor.extent.width, this->currentScissor.extent.height};

        auto colorAttachment = createColorAttachment(
            msaaColorBuffer.getImageView(), VK_ATTACHMENT_LOAD_OP_CLEAR, {0.0f, 0.0f, 0.5f, 0.0f},
            DeferredToCompute.getImageView(), VK_RESOLVE_MODE_AVERAGE_BIT);

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

        const auto descriptorSets =
            std::vector{
                this->SceneOptionsBoneDataSets[currentFrame].get(),
                this->materialDescriptorSet.get(),
                skyDescriptorSet.get(),
                shadowMapSet.get()};

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelines.at("pbrForward").getPipelineLayout(), 0,
                                static_cast<cUint32_t>(descriptorSets.size()),
                                descriptorSets.data(), 0, nullptr);

        for (size_t j = 0; j < this->currentModels->size(); j++)
        {
            if (!this->currentModels->at(j).Visible())
            {
                continue;
            }

            for (size_t i = 0; i < this->currentModels->at(j).Meshes().size(); i++)
            {

                auto &mesh = this->currentModels->at(j).Meshes()[i];

                // Skip culled meshes
                if (mesh.isCulled)
                {
                    continue;
                }

                cUint32_t matIndex = mesh.materialIndex;
                this->currentModels->at(j).ModelResource().materialIndex = matIndex;

                vkCmdPushConstants(cmd, pipelines.at("pbrForward").getPipelineLayout(),
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   sizeof(this->currentModels->at(j).ModelResource()), &this->currentModels->at(j).ModelResource());

                vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex->Buffer(), offsets);
                vkCmdBindIndexBuffer(cmd, mesh.index->Buffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd, static_cast<cUint32_t>(mesh.indices.size()), 1, 0, 0, 0);
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
            static_cast<cUint32_t>(skyDescriptorSets.size()), skyDescriptorSets.data(), 0, nullptr);
        vkCmdDraw(cmd, 36, 1, 0, 0);
        vkCmdEndRendering(cmd);
    }

    void VKRenderer::makePBRDeferredPass(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
        VkRect2D renderArea = {0, 0, this->currentScissor.extent.width, this->currentScissor.extent.height};

        auto colorAttachment = createColorAttachment(
            msaaColorBuffer.getImageView(), VK_ATTACHMENT_LOAD_OP_LOAD, {0.0f, 0.0f, 0.5f, 0.0f},
            DeferredToCompute.getImageView(), VK_RESOLVE_MODE_AVERAGE_BIT);

        auto depthAttachment =
            createDepthAttachment(
                msaaDepthStencil.getImageView(), VK_ATTACHMENT_LOAD_OP_DONT_CARE, 1.0f,
                depthStencil.getImageView(), VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);

        auto renderingInfo = createRenderingInfo(renderArea, &colorAttachment, &depthAttachment);

        vkCmdBeginRendering(cmd, &renderingInfo);
        vkCmdSetViewport(cmd, 0, 1, &this->currentViewport);
        vkCmdSetScissor(cmd, 0, 1, &this->currentScissor);

        VkDeviceSize offsets[1]{0};

        // Render models
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipelines.at("pbrdeferred").getPipeline());

        const auto descriptorSets =
            std::vector{
                this->SceneOptionsBoneDataSets[currentFrame].get(),
                this->materialDescriptorSet.get(),
                skyDescriptorSet.get(),
                shadowMapSet.get()};

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelines.at("pbrdeferred").getPipelineLayout(), 0,
                                static_cast<cUint32_t>(descriptorSets.size()),
                                descriptorSets.data(), 0, nullptr);

        for (size_t j = 0; j < this->currentModels->size(); j++)
        {
            if (!this->currentModels->at(j).Visible())
            {
                continue;
            }

            for (size_t i = 0; i < this->currentModels->at(j).Meshes().size(); i++)
            {

                auto &mesh = this->currentModels->at(j).Meshes()[i];

                // Skip culled meshes
                if (mesh.isCulled)
                {
                    continue;
                }

                cUint32_t matIndex = mesh.materialIndex;
                this->currentModels->at(j).ModelResource().materialIndex = matIndex;

                vkCmdPushConstants(cmd, pipelines.at("pbrdeferred").getPipelineLayout(),
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   sizeof(this->currentModels->at(j).ModelResource()), &this->currentModels->at(j).ModelResource());

                vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex->Buffer(), offsets);
                vkCmdBindIndexBuffer(cmd, mesh.index->Buffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd, static_cast<cUint32_t>(mesh.indices.size()), 1, 0, 0, 0);
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
            static_cast<cUint32_t>(skyDescriptorSets.size()), skyDescriptorSets.data(), 0, nullptr);
        vkCmdDraw(cmd, 36, 1, 0, 0);
        vkCmdEndRendering(cmd);
    }

    void VKRenderer::makeLightDeferredPass(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
        this->DeferredToCompute.transitionTo(cmd,
                                             VK_IMAGE_LAYOUT_GENERAL,
                                             VK_ACCESS_2_SHADER_READ_BIT,
                                             VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        // computeToPost_: Empty buffer → writeonly storage image for SSAO output
        this->LightDeferred.transitionTo(
            cmd,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_2_SHADER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        // Bind SSAO compute pipeline
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines.at("lightdeferred").getPipeline());

        // Bind descriptor sets for SSAO
        const auto ssaoDescriptorSets = std::vector{this->ssaoDescriptorSets[currentFrame].get()};
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipelines.at("lightdeferred").getPipelineLayout(), 0,
                                static_cast<cUint32_t>(ssaoDescriptorSets.size()),
                                ssaoDescriptorSets.data(), 0, nullptr);

        // Dispatch compute shader
        // Calculate dispatch size based on image dimensions and local work group size (16x16)
        cUint32_t groupCountX = (currentScissor.extent.width + 15) / 16;  // Round up division
        cUint32_t groupCountY = (currentScissor.extent.height + 15) / 16; // Round up division
        vkCmdDispatch(cmd, groupCountX, groupCountY, 1);

        VkMemoryBarrier2 memoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER_2};
        memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        memoryBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

        VkDependencyInfo dependencyInfo{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        dependencyInfo.memoryBarrierCount = 1;
        dependencyInfo.pMemoryBarriers = &memoryBarrier;
        vkCmdPipelineBarrier2(cmd, &dependencyInfo);
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
                                static_cast<cUint32_t>(postDescriptorSets.size()),
                                postDescriptorSets.data(), 0, nullptr);

        vkCmdDraw(cmd, 6, 1, 0, 0);
        vkCmdEndRendering(cmd);
    }

    void VKRenderer::makeSSAOPass(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
        this->DeferredToCompute.transitionTo(cmd,
                                             VK_IMAGE_LAYOUT_GENERAL,
                                             VK_ACCESS_2_SHADER_READ_BIT,
                                             VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        // computeToPost_: Empty buffer → writeonly storage image for SSAO output
        this->LightDeferred.transitionTo(
            cmd,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_2_SHADER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        // Bind SSAO compute pipeline
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines.at("ssao").getPipeline());

        // Bind descriptor sets for SSAO
        const auto ssaoDescriptorSets = std::vector{this->ssaoDescriptorSets[currentFrame].get()};
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipelines.at("ssao").getPipelineLayout(), 0,
                                static_cast<cUint32_t>(ssaoDescriptorSets.size()),
                                ssaoDescriptorSets.data(), 0, nullptr);

        // Dispatch compute shader
        // Calculate dispatch size based on image dimensions and local work group size (16x16)
        cUint32_t groupCountX = (currentScissor.extent.width + 15) / 16;  // Round up division
        cUint32_t groupCountY = (currentScissor.extent.height + 15) / 16; // Round up division
        vkCmdDispatch(cmd, groupCountX, groupCountY, 1);

        VkMemoryBarrier2 memoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER_2};
        memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        memoryBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

        VkDependencyInfo dependencyInfo{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
        dependencyInfo.memoryBarrierCount = 1;
        dependencyInfo.pMemoryBarriers = &memoryBarrier;
        vkCmdPipelineBarrier2(cmd, &dependencyInfo);
    }

    void VKRenderer::makeShadowMap(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
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
            static_cast<cUint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

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
                vkCmdDrawIndexed(cmd, static_cast<cUint32_t>(mesh.indices.size()), 1, 0, 0, 0);
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

    VkFormat VKRenderer::selectOptimalHDRFormat(cBool needsAlpha, cBool fullPrecision)
    {
        std::vector<VkFormat> candidateFormats;

        if (!needsAlpha && !fullPrecision)
        {
            // Memory-efficient HDR formats (no alpha, moderate precision)
            candidateFormats = {
                VK_FORMAT_B10G11R11_UFLOAT_PACK32, // 4 bytes - 50% savings, packed float (correct
                                                   // format)
                VK_FORMAT_R16G16B16_SFLOAT,        // 6 bytes - 25% savings, half precision
                VK_FORMAT_R16G16B16A16_SFLOAT,     // 8 bytes - standard HDR with alpha
                VK_FORMAT_R32G32B32_SFLOAT,        // 12 bytes - full precision RGB
                // R8G8B8A8_UNORM is LAST - not a float format, poor for HDR
                VK_FORMAT_R8G8B8A8_UNORM // 4 bytes - NOT FLOAT, last resort
            };
        }
        else if (!fullPrecision)
        {
            // Standard HDR with alpha channel
            candidateFormats = {
                VK_FORMAT_R16G16B16A16_SFLOAT, // 8 bytes - standard HDR
                VK_FORMAT_R32G32B32A32_SFLOAT, // 16 bytes - full precision
                // R8G8B8A8_UNORM is LAST - not suitable for HDR
                VK_FORMAT_R8G8B8A8_UNORM // 4 bytes - NOT FLOAT, last resort
            };
        }
        else
        {
            // Full precision required
            candidateFormats = {
                VK_FORMAT_R32G32B32A32_SFLOAT, // 16 bytes - full precision
                VK_FORMAT_R32G32B32_SFLOAT,    // 12 bytes - full precision RGB
                VK_FORMAT_R16G16B16A16_SFLOAT, // 8 bytes - half precision fallback
                // R8G8B8A8_UNORM is LAST - inadequate for full precision HDR
                VK_FORMAT_R8G8B8A8_UNORM // 4 bytes - NOT FLOAT, emergency fallback
            };
        }

        // Test each format for compatibility (float formats first, R8G8B8A8 last)
        for (size_t i = 0; i < candidateFormats.size(); ++i)
        {
            VkFormat format = candidateFormats[i];

            if (isFormatSuitableForHDR(format))
            {
                cString formatType = (format == VK_FORMAT_R8G8B8A8_UNORM) ? "NON-FLOAT" : "FLOAT";
                float memoryRatio = static_cast<float>(helper::shader::getFormatSize(format)) / 8.0f; // vs RGBA16F

                // Warn if we fell back to non-float format
                if (format == VK_FORMAT_R8G8B8A8_UNORM)
                {
                    PRINT_TO_LOGGER("⚠ WARNING: Using R8G8B8A8_UNORM for HDR - limited dynamic range!");
                    PRINT_TO_LOGGER("  Consider using float formats for better HDR quality");
                }

                return format;
            }
            else
            {
                cString formatType = (format == VK_FORMAT_R8G8B8A8_UNORM) ? "NON-FLOAT" : "FLOAT";
            }
        }

        // Emergency fallback - this should rarely happen
        PRINT_TO_LOGGER("⚠ All candidate formats failed, using emergency fallback: VK_FORMAT_R16G16B16A16_SFLOAT");

        return VK_FORMAT_R16G16B16A16_SFLOAT;
    }

    cBool VKRenderer::isFormatSuitableForHDR(VkFormat format)
    {
        // Check if format supports required features for HDR rendering
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(ctx.getDevice()->physicalDevice, format, &props);

        // Required features for HDR color attachments
        VkFormatFeatureFlags requiredFeatures =
            VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | // Can render to it
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;     // Can sample from it

        // Optional but preferred for HDR
        VkFormatFeatureFlags preferredFeatures =
            VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT; // Can blend (for transparency)

        bool hasRequired = (props.optimalTilingFeatures & requiredFeatures) == requiredFeatures;
        bool hasPreferred = (props.optimalTilingFeatures & preferredFeatures) == preferredFeatures;

        return hasRequired;
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