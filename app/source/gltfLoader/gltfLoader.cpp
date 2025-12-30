#include "gltfLoader.h"

using namespace vkengine::helper;
using namespace vkengine::debug;

using vkengine::object::Camera;

namespace vkengine {
    gltfLoaderEngine::gltfLoaderEngine(std::string root_path) : VulkanEngine(root_path) {}

    gltfLoaderEngine::~gltfLoaderEngine() {}

    void gltfLoaderEngine::init()
    {
        VulkanEngine::init();
        this->camera = std::make_shared<vkengine::object::Camera>(
            cVec3(0.0f, 0.0f, 1.0f), // 카메라 위치
            cVec3(0.0f, -1.0f, 0.0f), // 카메라 up
            cVec3(0.0f, 0.0f, 1.0f), // 카메라 dir
            cVec3(1.0f, 0.0f, 0.0f) // 카메라 right
        );
    }

    bool gltfLoaderEngine::prepare()
    {
        VulkanEngine::prepare();
        this->init_sync_structures();

        this->vkGUI = new vkengine::vkGUI(this);
        this->modelObject = new vkengine::object::GLTFmodelObject(this->getDevice());

        //cString path = "/glTF2/AntiqueCamera.gltf";
        cString path = "/noGit/sponza/glTF/Sponza.gltf";
        
        helper::loadModel::GLTF::loadAsset(path, this->modelObject);
        this->modelObject->setScale(cVec3(0.3f, 0.3f, 0.3f)); // 모델 크기 조정
        this->modelObject->updateMatrix();

        this->createVertexbuffer();
        this->createIndexBuffer();
        this->createUniformBuffers();

        this->createDescriptor();
        this->createGraphicsPipeline();

        this->initUI();

        return true;
    }

    void gltfLoaderEngine::cleanup()
    {
        if (this->_isInitialized)
        {
            this->vkGUI->cleanup();
            this->cleanupSwapcChain();

            vkDestroyRenderPass(this->VKdevice->logicaldevice, *this->VKrenderPass.get(), nullptr);
            vkDestroyDescriptorPool(this->VKdevice->logicaldevice, this->VKdescriptorPool, nullptr);

            if (this->VKgraphicsPipeline) {
                vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKgraphicsPipeline, nullptr);
                this->VKgraphicsPipeline = VK_NULL_HANDLE;
            }
            if (this->pipelineLayout) {
                vkDestroyPipelineLayout(this->VKdevice->logicaldevice, this->pipelineLayout, nullptr);
                this->pipelineLayout = VK_NULL_HANDLE;
            }

            for (auto& pair : this->manager.setLayouts) {
                if (pair.second.layout) {
                    vkDestroyDescriptorSetLayout(this->VKdevice->logicaldevice, pair.second.layout, nullptr);
                    pair.second.layout = VK_NULL_HANDLE;
                }
            }
            
            if (this->manager.pool) {
                vkDestroyDescriptorPool(this->VKdevice->logicaldevice, this->manager.pool, nullptr);
                this->manager.pool = VK_NULL_HANDLE;
            }

            this->shaderData.buffer.cleanup();
            this->modelObject->cleanup();

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroySemaphore(this->VKdevice->logicaldevice, this->VKframeData[i].VkimageavailableSemaphore, nullptr);
                vkDestroySemaphore(this->VKdevice->logicaldevice, this->VKframeData[i].VkrenderFinishedSemaphore, nullptr);
                vkDestroyFence(this->VKdevice->logicaldevice, this->VKframeData[i].VkinFlightFences, nullptr);
            }

            vkDestroyPipelineCache(this->VKdevice->logicaldevice, this->VKpipelineCache, nullptr);

            this->VKdevice->cleanup();

            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(this->VKinstance, this->VKdebugUtilsMessenger, nullptr);
            }

            vkDestroySurfaceKHR(this->VKinstance, this->VKsurface, nullptr);
            vkDestroyInstance(this->VKinstance, nullptr);

            glfwDestroyWindow(this->VKwindow);
            glfwTerminate();
        }
    }

    void gltfLoaderEngine::drawFrame()
    {
        _VK_CHECK_RESULT_(vkWaitForFences(this->VKdevice->logicaldevice, 1, &this->getCurrnetFrameData().VkinFlightFences, VK_TRUE, UINT64_MAX));

        uint32_t imageIndex = 0;
        VulkanEngine::prepareFame(&imageIndex);

        this->shaderData.values.projection = this->camera->getProjectionMatrix();
        this->shaderData.values.model = this->camera->getViewMatrix();
        this->shaderData.values.viewPos = cVec4(this->camera->getPos(), 0.0f);
        memcpy(this->shaderData.buffer.mapped, &this->shaderData.values, sizeof(this->shaderData.values));

        vkResetFences(this->VKdevice->logicaldevice, 1, &this->getCurrnetFrameData().VkinFlightFences);
        vkResetCommandBuffer(this->VKframeData[this->currentFrame].mainCommandBuffer, 0);
        this->recordCommandBuffer(&this->getCurrnetFrameData(), imageIndex);

        VKsubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
        VkSemaphore waitSemaphores[] = { this->VKframeData[this->currentFrame].VkimageavailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VKsubmitInfo.waitSemaphoreCount = 1;
        VKsubmitInfo.pWaitSemaphores = waitSemaphores;
        VKsubmitInfo.pWaitDstStageMask = waitStages;
        VKsubmitInfo.commandBufferCount = 1;
        VKsubmitInfo.pCommandBuffers = &this->VKframeData[this->currentFrame].mainCommandBuffer;

        VkSemaphore signalSemaphores[] = { this->VKframeData[this->currentFrame].VkrenderFinishedSemaphore };
        VKsubmitInfo.signalSemaphoreCount = 1;
        VKsubmitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(this->VKdevice->logicaldevice, 1, &this->VKframeData[this->currentFrame].VkinFlightFences);
        _VK_CHECK_RESULT_(vkQueueSubmit(this->VKdevice->graphicsVKQueue, 1, &VKsubmitInfo, this->VKframeData[this->currentFrame].VkinFlightFences));

        VulkanEngine::presentFrame(&imageIndex);

        this->currentFrame = (this->currentFrame + 1) % this->frames;
    }

    bool gltfLoaderEngine::mainLoop()
    {
        while (!glfwWindowShouldClose(this->VKwindow)) {
            glfwPollEvents();
            float time = this->getProgramRunTime();

            this->update(time);

            this->vkGUI->begin();
            this->vkGUI->update();

            ImGui::Text("Camera Yaw, Pitch");
            ImGui::Text("Yaw: %.4f, Pitch: %.4f", this->getCamera()->getYaw(), this->getCamera()->getPitch());

            bool check = this->getCameraMoveStyle();
            ImGui::Checkbox("Camera Move Style", &check);
            this->setCameraMoveStyle(check);

            float fov = this->getCamera()->getFov();
            ImGui::Text("Camera Fov: %.4f", fov);

            this->vkGUI->end();

            drawFrame();


        }

        vkDeviceWaitIdle(this->VKdevice->logicaldevice);
        state = false;

        return state;
    }

    void gltfLoaderEngine::update(float dt)
    {
        float frameTime = this->getCalulateDeltaTime();

        if (this->m_keyPressed[GLFW_KEY_W])
        {
            this->camera->MoveForward(frameTime * MOVESPEED);
        }

        if (this->m_keyPressed[GLFW_KEY_S])
        {
            this->camera->MoveForward(-frameTime * MOVESPEED);
        }

        if (this->m_keyPressed[GLFW_KEY_A])
        {
            this->camera->MoveRight(-frameTime * MOVESPEED);
        }

        if (this->m_keyPressed[GLFW_KEY_D])
        {
            this->camera->MoveRight(frameTime * MOVESPEED);
        }

        if (this->m_keyPressed[GLFW_KEY_Q])
        {
            this->camera->MoveUp(-frameTime * MOVESPEED);
        }

        if (this->m_keyPressed[GLFW_KEY_E])
        {
            this->camera->MoveUp(frameTime * MOVESPEED);
        }

        this->camera->update();

        float aspectRatio = static_cast<float>(this->VKswapChain->getSwapChainExtent().width) / static_cast<float>(VKswapChain->getSwapChainExtent().height);

        float fov = this->camera->getFov();
        float nearP = this->camera->getNearP();
        float farP = this->camera->getFarP();

        this->camera->setPerspectiveProjection(fov, aspectRatio, nearP, farP);
    }

    bool gltfLoaderEngine::init_sync_structures()
    {
        VkSemaphoreCreateInfo semaphoreInfo = helper::semaphoreCreateInfo(0);
        VkFenceCreateInfo fenceInfo = helper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        for (auto& frameData : this->VKframeData)
        {
            _VK_CHECK_RESULT_(vkCreateSemaphore(this->VKdevice->logicaldevice, &semaphoreInfo, nullptr, &frameData.VkimageavailableSemaphore));
            _VK_CHECK_RESULT_(vkCreateSemaphore(this->VKdevice->logicaldevice, &semaphoreInfo, nullptr, &frameData.VkrenderFinishedSemaphore));
        }

        return true;
    }

    void gltfLoaderEngine::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo = framedata->commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // 렌더 패스를 시작하기 위한 클리어 값 설정
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.4f, 0.4f, 0.4f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        // 렌더 패스 시작 정보 구조체를 초기화합니다.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = *this->VKrenderPass.get();
        renderPassInfo.framebuffer = this->VKswapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = this->VKswapChain->getSwapChainExtent();
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
        renderPassInfo.pClearValues = clearValues.data();

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(this->VKswapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(this->VKswapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = this->VKswapChain->getSwapChainExtent();

        _VK_CHECK_RESULT_(vkBeginCommandBuffer(framedata->mainCommandBuffer, &beginInfo));

        vkCmdBeginRenderPass(framedata->mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdSetViewport(framedata->mainCommandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(framedata->mainCommandBuffer, 0, 1, &scissor);
            vkCmdBindDescriptorSets(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1, &this->matrixDescritor.descriptorSet, 0, nullptr);
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
            this->drawGLTF(framedata->mainCommandBuffer, this->pipelineLayout);

            this->vkGUI->render();
        }
        vkCmdEndRenderPass(framedata->mainCommandBuffer);

        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void gltfLoaderEngine::createVertexbuffer()
    {
        this->modelObject->createVertexBuffer(*this->modelObject->getVertices());
    }

    void gltfLoaderEngine::createIndexBuffer()
    {
        this->modelObject->createIndexBuffer(*this->modelObject->getIndices());
    }

    void gltfLoaderEngine::createUniformBuffers()
    {
        this->shaderData.buffer.device = this->VKdevice->logicaldevice;
        this->shaderData.buffer.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        this->shaderData.buffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        this->shaderData.buffer.createBuffer(this->VKdevice->physicalDevice);
        this->shaderData.buffer.mapToMeBuffer(this->shaderData.buffer.size, 0);
        this->shaderData.buffer.createDescriptorBufferInfo();

        this->modelObject->getTexture()->createDescriptorImageInfo();
    }

    // 임시로 디스크립터를 생성하는 함수
    // TODO : 디스크립터를 생성하는 함수를 작성해야함
    void gltfLoaderEngine::createDescriptor()
    {
        manager = DescriptorManager(this->VKdevice->logicaldevice);

        // pool create
        std::vector<VkDescriptorPoolSize> modelPoolSizes =
        {
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<cUint32_t>(this->modelObject->getTexture()->imageData.size()))
        };
        cUint32_t maxSetCount = static_cast<uint32_t>(this->modelObject->getTexture()->imageData.size()) + 1;
        VkDescriptorPoolCreateInfo poolInfo = helper::descriptorPoolCreateInfo(modelPoolSizes, maxSetCount);
        manager.createDescriptorPool(poolInfo);

        // setLayouts create
        manager.setLayouts.reserve(2);

        VkDescriptorSetLayoutBinding setLayoutBindingsModel = helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
        manager.setLayouts.insert({ "matrix", setLayoutBinding(setLayoutBindingsModel) });
        manager.setLayouts["matrix"].createInfo(1); // createInfo()를 호출하여 layoutInfo를 설정합니다.

        manager.createDescriptorSetLayouts("matrix");
        setLayoutBindingsModel = helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
        manager.setLayouts.insert({ "textures", setLayoutBinding(setLayoutBindingsModel) });
        manager.setLayouts["textures"].createInfo(1); // createInfo()를 호출하여 layoutInfo를 설정합니다.
        
        manager.createDescriptorSetLayouts("textures");

        // Descriptor set layout for matrices
        std::vector<VkDescriptorSetLayout> layouts(1, this->manager.setLayouts["matrix"].layout);
        VkDescriptorSetAllocateInfo allocInfo = helper::descriptorSetAllocateInfo(this->manager.pool, *layouts.data(), 1);
        _VK_CHECK_RESULT_(vkAllocateDescriptorSets(this->VKdevice->logicaldevice, &allocInfo, &this->matrixDescritor.descriptorSet));
        VkWriteDescriptorSet writeDescriptorSet01 = helper::writeDescriptorSet(this->matrixDescritor.descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &this->shaderData.buffer.descriptor);
        vkUpdateDescriptorSets(this->VKdevice->logicaldevice, 1, &writeDescriptorSet01, 0, nullptr);

        // Descriptor set layout for textures
        // layout과 descriptorset가 다르기 때문에, 각각의 이미지에 대해 descriptorSet을 생성해야 한다.

        int index = 0;
        for (auto& image : this->modelObject->getTexture()->imageData)
        {
            std::vector<VkDescriptorSetLayout> layouts2(1, this->manager.setLayouts["textures"].layout);
            allocInfo = helper::descriptorSetAllocateInfo(this->manager.pool, *layouts2.data(), 1);
            _VK_CHECK_RESULT_(vkAllocateDescriptorSets(this->VKdevice->logicaldevice, &allocInfo, &image.descriptorSet));
            writeDescriptorSet01 = helper::writeDescriptorSet(image.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &image.imageInfo);
            vkUpdateDescriptorSets(this->VKdevice->logicaldevice, 1, &writeDescriptorSet01, 0, nullptr);
            index++;
        }

    }

    void gltfLoaderEngine::createGraphicsPipeline()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertmesh.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragmesh.spv");

        VkPipelineShaderStageCreateInfo shaderStages[2] = {
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, baseVertshaderModule, "main"),
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, baseFragShaderModule, "main")
        };

        // vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = helper::pipelineVertexInputStateCreateInfo(
            bindingDescription, *attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()), 1);
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = helper::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

        VkViewport viewpport{};
        viewpport.x = 0.0f;
        viewpport.y = 0.0f;
        viewpport.width = (float)this->VKswapChain->getSwapChainExtent().width;
        viewpport.height = (float)this->VKswapChain->getSwapChainExtent().height;
        viewpport.minDepth = 0.0f;
        viewpport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = VKswapChain->getSwapChainExtent();

        VkPipelineViewportStateCreateInfo viewportState = helper::pipelineViewportStateCreateInfo(viewpport, scissor, 1, 1);
        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
        VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
        VkPipelineDynamicStateCreateInfo dynamicState = helper::pipelineDynamicStateCreateInfo(dynamicStates);

        std::vector<VkPushConstantRange> pushConstantRanges = {
            helper::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(cMat4), 0), // 푸시 상수 범위 설정
            helper::pushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(cFloat), sizeof(cMat4)) // texture index 사용 -> sampler array를 사용하기 때문에
        };
#if 0
        std::array<VkDescriptorSetLayout, 2> setLayouts = { descriptorSetLayouts.matrices, descriptorSetLayouts.textures };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = helper::pipelineLayoutCreateInfo(setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));

        pipelineLayoutInfo.pushConstantRangeCount = 2;
        pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

        _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutInfo, nullptr, &this->pipelineLayout));
#else
        std::array<VkDescriptorSetLayout, 2> setLayouts = { this->manager.setLayouts["matrix"].layout, this->manager.setLayouts["textures"].layout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = helper::pipelineLayoutCreateInfo(setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));

        pipelineLayoutInfo.pushConstantRangeCount = 2;
        pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

        _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutInfo, nullptr, &this->pipelineLayout));
#endif
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState; // Optional
        pipelineInfo.layout = this->pipelineLayout;
        pipelineInfo.renderPass = *this->VKrenderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, this->VKpipelineCache, 1, &pipelineInfo, nullptr, &this->VKgraphicsPipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);
    }

    void gltfLoaderEngine::initUI()
    {
        this->vkGUI->init(200, 200);
        this->vkGUI->initResources(this->getRenderPass(), this->getDevice()->graphicsVKQueue, this->getRootPath());
    }

    void gltfLoaderEngine::cleanupSwapcChain()
    {
        this->VKdepthStencill.cleanup(this->VKdevice->logicaldevice);

        for (auto framebuffers : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffers, nullptr);
        }

        this->VKswapChain->cleanupSwapChain();
    }

    void gltfLoaderEngine::drawGLTF(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
    {
        // All vertices and indices are stored in single buffers, so we only need to bind once
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->modelObject->getVertexBuffer()->buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, this->modelObject->getIndexBuffer()->buffer, 0, VK_INDEX_TYPE_UINT32);
        // Render all nodes at top-level

        for (auto node : this->modelObject->getNodes()) {
            drawNode(commandBuffer, pipelineLayout, node, this->modelObject->getMatrix());
        }
    }

    void gltfLoaderEngine::drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Node* node, cMat4 modelObjectMatrix)
    {
        if (node->mesh.primitives.size() > 0) {
            cMat4 nodeMatrix = node->matrix;
            Node* currentParent = node->parent;

            while (currentParent) {
                nodeMatrix = currentParent->matrix * nodeMatrix;
                currentParent = currentParent->parent;
            }
            // Pass the final matrix to the vertex shader using push constants
            nodeMatrix = modelObjectMatrix * nodeMatrix; // Combine with the model object matrix
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
            for (Primitive& primitive : node->mesh.primitives) {
               if (primitive.indexCount > 0) {
                    // Get the texture index for this primitive
                    //Primitive& primitive = node->mesh.primitives[0]; // Assuming we only want to draw the first primitive for simplicity
                    cFloat texture = static_cast<cFloat>(this->modelObject->textureIndexes[this->modelObject->materials[primitive.materialIndex].baseColorTextureIndex]);
                    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), sizeof(cUint32_t), &texture);

                    // Bind the descriptor for the current primitive's texture
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &this->modelObject->getTexture()->imageData[static_cast<cInt>(texture)].descriptorSet, 0, nullptr);
                    vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
                }
            }
        }
        
        for (auto& child : node->children) {
            drawNode(commandBuffer, pipelineLayout, child, modelObjectMatrix);
        }
    }



}