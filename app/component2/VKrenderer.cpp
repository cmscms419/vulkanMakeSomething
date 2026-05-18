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
        VKSwapChain &swapchain,
        const cUint32_t &MaxFramesFlight,
        const cString &assetsPath,
        const cString &shaderPath)
        : ctx(ctx), renderGraph(ctx, swapchain), shaderManager(shadermanager),
          MaxFramesFlight(MaxFramesFlight), assetsPath(assetsPath), shaderPath(shaderPath),
          samplerLinearRepeat(ctx), samplerLinearClamp(ctx), samplerAnisoRepeat(ctx), samplerAnisoClamp(ctx),
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

    void VKRenderer::buildRenderGraph()
    {
        this->renderGraph.registerResource("shadowDepth", this->images["shadowMap"]);               // 쉐도우 맵 리소스 등록
        this->renderGraph.registerResource("DeferredToCompute", this->images["DeferredToCompute"]); // 포워드 패스 출력 등록
        this->renderGraph.registerResource("LightDeferred", this->images["LightDeferred"]);         // 컴퓨트 패스 출력 등록
        this->renderGraph.registerResource("depthStencil", this->images["depthStencil"]);           // depthstencil 리소스 등록

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

        this->renderGraph.registerPassFunction("skybox",
                                               [this](VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageIndex)
                                               {
                                                   this->makeSkyboxProcessPass(cmd, frameIndex, imageIndex);
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

    void VKRenderer::prepareForModels(std::vector<VKModel> &models, VkFormat outColorFormat, VkFormat depthFormat, cUint32_t swapChainWidth, cUint32_t swapChainHeight)
    {
        this->createPipelines(outColorFormat, depthFormat);
        this->createTextures(swapChainWidth, swapChainHeight);
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

    void VKRenderer::createPipelines(const VkFormat colorFormat, const VkFormat depthFormat)
    {
        VkFormat selectedHDRFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        pipelines.emplace("pbrdeferred",
                          VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createPbrDeferred(),
                                           std::vector<VkFormat>{VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                 VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT},
                                           depthFormat, VK_SAMPLE_COUNT_1_BIT));
        pipelines.emplace("sky", VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createSky(), std::vector<VkFormat>{selectedHDRFormat},
                                                  depthFormat, VK_SAMPLE_COUNT_1_BIT));
        pipelines.emplace("post", VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createPost(), std::vector<VkFormat>{colorFormat},
                                                   depthFormat, VK_SAMPLE_COUNT_1_BIT));
        pipelines.emplace("shadowMap", VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createShadowMap(), std::vector<VkFormat>{},
                                                        VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT));
        pipelines.emplace("lightdeferred", VKPipeLineHandle(ctx, shaderManager, PipelineConfig::createDeferredLighting(), std::vector<VkFormat>{},
                                                            std::nullopt, VK_SAMPLE_COUNT_1_BIT));
    }

    void VKRenderer::createTextures(cUint32_t swapchainWidth, cUint32_t swapchainHeight)
    {
        this->samplerLinearRepeat.createLinearRepeat();
        this->samplerLinearClamp.createLinearClamp();
        this->samplerAnisoRepeat.createAnisoRepeat();
        this->samplerAnisoClamp.createAnisoClamp();
        this->samplerShadowMap.createShadowMapSampler();

        // Initialize image buffers (simplified - no MSAA)
        const std::vector<cString> imageNames = {
            "gAlbedo", "gNormal", "gPosition",
            "gMaterial", "shadowMap", "prefiltered",
            "irradiance", "brdfLUT", "depthStencil",
            "DeferredToCompute", "LightDeferred"};

        for (const auto &name : imageNames)
        {
            this->images[name] = std::make_shared<VKImage2D>(ctx);
        }

        // Initialize shadow map texture
        this->images["shadowMap"]->createShadowMap(2048 * 2, 2048 * 2);
        this->images["shadowMap"]->setSampler(this->samplerShadowMap.getSampler());

        // Initialize IBL textures for PBR
        cString path = this->assetsPath + "cubeMap/";
        this->images["prefiltered"]->createTextureFromKtx2(path + "specular_out.ktx2", true);
        this->images["irradiance"]->createTextureFromKtx2(path + "diffuse_out.ktx2", true);
        this->images["brdfLUT"]->createTextureFromImage(path + "outputLUT.png", false, true);

        this->images["prefiltered"]->setSampler(this->samplerLinearRepeat.getSampler());
        this->images["irradiance"]->setSampler(this->samplerLinearRepeat.getSampler());
        this->images["brdfLUT"]->setSampler(this->samplerLinearClamp.getSampler());

        // Create render targets
        this->images["depthStencil"]->createDepthStencil(swapchainWidth, swapchainHeight, true);
        this->images["DeferredToCompute"]->createGeneralStorage(swapchainWidth, swapchainHeight);
        this->images["LightDeferred"]->createGeneralStorage(swapchainWidth, swapchainHeight);

        // Set samplers
        this->images["DeferredToCompute"]->setSampler(samplerLinearRepeat.getSampler());
        this->images["depthStencil"]->setSampler(samplerLinearClamp.getSampler());
        this->images["LightDeferred"]->setSampler(samplerLinearRepeat.getSampler());

        // Create descriptor sets for sky textures (set 1 for sky pipeline)
        skyDescriptorSet.create(ctx, {std::ref(*this->images["prefiltered"]),
                                      std::ref(*this->images["irradiance"]),
                                      std::ref(*this->images["brdfLUT"])});

        // Create descriptor set for shadow mapping
        shadowMapSet.create(ctx, {std::ref(*this->images["shadowMap"])});

        // Create G-buffer textures for deferred rendering
        PRINT_TO_LOGGER("Creating G-buffer textures for deferred rendering:");

        // G-Buffer format selection for optimal memory usage and precision
        VkFormat albedoFormat = VK_FORMAT_R8G8B8A8_UNORM;        // Albedo + Metallic (4 bytes)
        VkFormat normalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;   // Normal + Roughness (8 bytes, needs precision)
        VkFormat positionFormat = VK_FORMAT_R32G32B32A32_SFLOAT; // Position + Depth (16 bytes, needs high precision)
        VkFormat materialFormat = VK_FORMAT_R16G16B16A16_SFLOAT; // Emissive RGB + AO (HDR emissive needs float)

        // G-buffer usage flags (similar to floatColor but without storage bit since they're render targets)
        VkImageUsageFlags gBufferUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                         VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VkDevice logicaldevice = ctx.getDevice()->logicaldevice;
        VkPhysicalDevice physicalDevice = ctx.getDevice()->physicalDevice;

        // Create gAlbedo buffer (Albedo RGB + Metallic A)
        images["gAlbedo"]->createImage(
            swapchainWidth,
            swapchainHeight,
            albedoFormat,
            VK_SAMPLE_COUNT_1_BIT,
            gBufferUsage,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);
        images["gAlbedo"]->setSampler(samplerLinearClamp.getSampler());

        // Create gNormal buffer (World Normal RGB + Roughness A)
        images["gNormal"]->createImage(
            swapchainWidth,
            swapchainHeight,
            normalFormat,
            VK_SAMPLE_COUNT_1_BIT,
            gBufferUsage,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);
        images["gNormal"]->setSampler(samplerLinearClamp.getSampler());

        // Create gPosition buffer (World Position RGB + Depth A)
        images["gPosition"]->createImage(
            swapchainWidth,
            swapchainHeight,
            positionFormat,
            VK_SAMPLE_COUNT_1_BIT,
            gBufferUsage,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);
        images["gPosition"]->setSampler(samplerLinearClamp.getSampler());

        // Create gMaterial buffer (AO R + Emissive Intensity G + Material ID B + Unused A)
        images["gMaterial"]->createImage(
            swapchainWidth,
            swapchainHeight,
            materialFormat,
            VK_SAMPLE_COUNT_1_BIT,
            gBufferUsage,
            VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0);
        images["gMaterial"]->setSampler(samplerLinearClamp.getSampler());

        // Register G-buffer images in renderGraph for automatic layout transitions
        for (const auto &name : {"gAlbedo", "gNormal", "gPosition", "gMaterial"})
        {
            this->renderGraph.registerResource(name, this->images.at(name));
        }

        PRINT_TO_LOGGER("G-buffer creation complete");
    }

    void VKRenderer::resize(cUint32_t width, cUint32_t height)
    {
        // 크기에 의존하는 이미지 정리
        this->images["depthStencil"]->cleanup();
        this->images["DeferredToCompute"]->cleanup();
        this->images["LightDeferred"]->cleanup();

        // 새 크기로 재생성
        this->images["depthStencil"]->createDepthStencil(width, height, VK_SAMPLE_COUNT_1_BIT);
        this->images["DeferredToCompute"]->createGeneralStorage(width, height);
        this->images["LightDeferred"]->createGeneralStorage(width, height);

        this->images["DeferredToCompute"]->setSampler(this->samplerLinearRepeat.getSampler());

        // PostDescriptorSets는 forwardToCompute를 참조하므로 재생성
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            PostDescriptorSets[i].create(
                this->ctx, {std::ref(*this->images["DeferredToCompute"]),
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
                this->ctx, {std::ref(*this->images["LightDeferred"]), std::ref(postOptionsUniform[i].Buffer())});
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

        this->images["DeferredToCompute"]->transitionTo(
            cmd.getCommandBuffer(),
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_2_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        this->images["LightDeferred"]->transitionTo(
            cmd.getCommandBuffer(),
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_2_SHADER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        cmd.submitAndWait();
        lightDeferredDescriptorSets.resize(this->MaxFramesFlight);
        for (size_t i = 0; i < this->MaxFramesFlight; i++)
        {
            lightDeferredDescriptorSets[i].create(this->ctx, {std::ref(sceneDataUniform[i].Buffer()),
                                                              std::ref(optionsUniform[i].Buffer()),
                                                              std::ref(ssaoParamsUniform[i].Buffer()),
                                                              std::ref(*this->images["DeferredToCompute"]),
                                                              std::ref(*this->images["LightDeferred"]),
                                                              std::ref(*this->images["depthStencil"]),
                                                              std::ref(*this->images["gAlbedo"]),
                                                              std::ref(*this->images["gNormal"]),
                                                              std::ref(*this->images["gPosition"]),
                                                              std::ref(*this->images["gMaterial"]),
                                                              std::ref(*this->images["shadowMap"]),
                                                              std::ref(*this->images["prefiltered"]),
                                                              std::ref(*this->images["irradiance"]),
                                                              std::ref(*this->images["brdfLUT"])});
        }
    }

    void VKRenderer::makePBRDeferredPass(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
        VkRect2D renderArea = {0, 0, this->currentScissor.extent.width, this->currentScissor.extent.height};

        std::vector<VkRenderingAttachmentInfo> colorAttachments{};
        VkRenderingAttachmentInfo depthAttachment = createDepthAttachment(this->images["depthStencil"]->getImageView(), VK_ATTACHMENT_LOAD_OP_CLEAR, 1.0f);
        colorAttachments.push_back(createColorAttachment(this->images["gAlbedo"]->getImageView(), VK_ATTACHMENT_LOAD_OP_CLEAR));
        colorAttachments.push_back(createColorAttachment(this->images["gNormal"]->getImageView(), VK_ATTACHMENT_LOAD_OP_CLEAR));
        colorAttachments.push_back(createColorAttachment(this->images["gPosition"]->getImageView(), VK_ATTACHMENT_LOAD_OP_CLEAR));
        colorAttachments.push_back(createColorAttachment(this->images["gMaterial"]->getImageView(), VK_ATTACHMENT_LOAD_OP_CLEAR));

        VkRenderingInfo renderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
        renderingInfo.renderArea = renderArea;
        renderingInfo.layerCount = 1;
        if (colorAttachments.size() > 1)
        {
            renderingInfo.colorAttachmentCount = static_cast<cUint32_t>(colorAttachments.size());
            renderingInfo.pColorAttachments = colorAttachments.data();
        }
        renderingInfo.pDepthAttachment = &depthAttachment;
        // renderingInfo.pStencilAttachment = &depthAttachment;

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
                this->materialDescriptorSet.get()};

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
        vkCmdEndRendering(cmd);
    }

    void VKRenderer::makeLightDeferredPass(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
        this->images["DeferredToCompute"]->transitionTo(cmd,
                                                        VK_IMAGE_LAYOUT_GENERAL,
                                                        VK_ACCESS_2_SHADER_READ_BIT,
                                                        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        // computeToPost_: Empty buffer → writeonly storage image for SSAO output
        this->images["LightDeferred"]->transitionTo(
            cmd,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_2_SHADER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        // G-buffer images: COLOR_ATTACHMENT_OPTIMAL → SHADER_READ_ONLY_OPTIMAL for compute shader sampling
        for (const auto &name : {"gAlbedo", "gNormal", "gPosition", "gMaterial"})
        {
            this->images.at(name)->transitionTo(cmd,
                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                VK_ACCESS_2_SHADER_READ_BIT,
                                                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);
        }

        this->images["depthStencil"]->transitionTo(cmd,
                                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                   VK_ACCESS_2_SHADER_READ_BIT,
                                                   VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        this->images["shadowMap"]->transitionTo(cmd,
                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                VK_ACCESS_2_SHADER_READ_BIT,
                                                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        // Bind SSAO compute pipeline
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines.at("lightdeferred").getPipeline());

        // Bind descriptor sets for SSAO
        const auto DescriptorSets = std::vector{this->lightDeferredDescriptorSets[currentFrame].get()};
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipelines.at("lightdeferred").getPipelineLayout(), 0,
                                static_cast<cUint32_t>(DescriptorSets.size()),
                                DescriptorSets.data(), 0, nullptr);

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
        // auto renderingInfo = createRenderingInfo(renderArea, &colorAttachment, nullptr);

        VkRenderingInfo renderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
        renderingInfo.renderArea = renderArea;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = nullptr;
        renderingInfo.pStencilAttachment = nullptr;

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

    void VKRenderer::makeSkyboxProcessPass(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
        VkRect2D renderArea = {0, 0, this->currentScissor.extent.width, this->currentScissor.extent.height};

        // LOAD: G-buffer 위에 sky 추가
        // VK_ATTACHMENT_LOAD_OP_LOAD는 Vulkan VkAttachmentLoadOp 열거형 값(숫자 0)으로, 첨부 파일의 기존 콘텐츠를 보존하고 렌더링 패스 시작 시 사용할 수 있도록 해야 함을 의미합니다.
        VkRenderingAttachmentInfo skyColorAttachment = createColorAttachment(this->images["DeferredToCompute"]->getImageView(), VK_ATTACHMENT_LOAD_OP_LOAD);
        VkRenderingAttachmentInfo skyDepthAttachment = createDepthAttachment(this->images["depthStencil"]->getImageView(), VK_ATTACHMENT_LOAD_OP_LOAD);

        VkRenderingInfo skyRenderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
        skyRenderingInfo.renderArea = renderArea;
        skyRenderingInfo.layerCount = 1;
        skyRenderingInfo.colorAttachmentCount = 1;
        skyRenderingInfo.pColorAttachments = &skyColorAttachment;
        skyRenderingInfo.pDepthAttachment = &skyDepthAttachment;

        vkCmdBeginRendering(cmd, &skyRenderingInfo);
        vkCmdSetViewport(cmd, 0, 1, &this->currentViewport);
        vkCmdSetScissor(cmd, 0, 1, &this->currentScissor);

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

    void VKRenderer::makeShadowMap(VkCommandBuffer cmd, cUint32_t currentFrame, cUint32_t imageIndex)
    {
#if 1
        // 그림자 맵 렌더링 시작
        VkRenderingAttachmentInfo shadowDepthAttachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
        shadowDepthAttachment.imageView = this->images["shadowMap"]->getImageView();
        shadowDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        shadowDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        shadowDepthAttachment.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo shadowRenderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
        shadowRenderingInfo.renderArea = {0, 0, this->images["shadowMap"]->getWidth(), this->images["shadowMap"]->getHeight()};
        shadowRenderingInfo.layerCount = 1;
        shadowRenderingInfo.colorAttachmentCount = 0;
        shadowRenderingInfo.pDepthAttachment = &shadowDepthAttachment;

        VkViewport shadowViewport{0.0f, 0.0f, (float)this->images["shadowMap"]->getWidth(), (float)this->images["shadowMap"]->getHeight(),
                                  0.0f, 1.0f};
        VkRect2D shadowScissor{0, 0, this->images["shadowMap"]->getWidth(), this->images["shadowMap"]->getHeight()};

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

    void VKRenderer::updateBoneData(const std::vector<VKModel>& models, uint32_t currentFrame)
    {
        PRINT_TO_LOGGER("Updating bone data for %zu models", models.size());

        // Reset bone data
        boneDataUBO.animationData.x = 0.0f;
        for (int i = 0; i < MAX_BONES; ++i)
        {
            boneDataUBO.boneMatrices[i] = glm::mat4(1.0f);
        }

        // Check if any model has animation data
        bool hasAnyAnimation = false;
        for (const auto &model : models)
        {
            if (model.hasAnimations() && model.hasBones())
            {
                hasAnyAnimation = true;

                // Get bone matrices from the first animated model
                const auto &boneMatrices = model.getBoneMatrices();

                // Copy bone matrices (up to data.h MAX_BONES) to UBO
                const size_t maxBones = MAX_BONES;
                size_t bonesToCopy = (boneMatrices.size() < maxBones) ? boneMatrices.size() : maxBones;
                for (size_t i = 0; i < bonesToCopy; ++i)
                {
                    boneDataUBO.boneMatrices[i] = boneMatrices[i];
                }

                break; // For now, use the first animated model
            }
        }

        boneDataUBO.animationData.x = float(hasAnyAnimation);

        // DEBUG: Log hasAnimation state
        static bool lastHasAnimation = false;
        if (lastHasAnimation != hasAnyAnimation)
        {
            PRINT_TO_LOGGER("hasAnimation changed to: %d", hasAnyAnimation);
            lastHasAnimation = hasAnyAnimation;
        }

        // Update the GPU buffer using the consolidated map structure
        boneDataUniform[currentFrame].updateData();
    }
}