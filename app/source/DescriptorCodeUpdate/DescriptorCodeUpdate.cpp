#include "DescriptorCodeUpdate.h"


using namespace vkengine::helper;
using namespace vkengine::debug;

using vkengine::object::Camera;

namespace vkengine {

    DescriptorCodeUpdateEngine::DescriptorCodeUpdateEngine(std::string root_path) : VulkanEngine(root_path) {}

    DescriptorCodeUpdateEngine::~DescriptorCodeUpdateEngine() {}

    void DescriptorCodeUpdateEngine::init()
    {
        VulkanEngine::init();
        this->camera = std::make_shared<vkengine::object::Camera>();
    }

    bool DescriptorCodeUpdateEngine::prepare()
    {
        VulkanEngine::prepare();
        this->init_sync_structures();

        this->cubeSkybox = new object::SkyBox(this->getDevice());

        std::vector<std::string> pathCubeArray = {
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/right.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/left.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/top.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/bottom.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/front.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/back.png"
        };

        for (auto& path : pathCubeArray)
        {
            TextureResource* resource = new TextureResource();
            resource->createResource(path);

            if (resource->data == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to load texture from %s\n", path.c_str());
                return false;
            }

            this->cubeSkybox->setResource(resource);

        }
        this->cubeSkybox->createTexture(VK_FORMAT_R8G8B8A8_SRGB);

        this->vkGUI = new vkengine::vkGUI(this);

        TextureResource* vikingRoomTexture = new TextureResource();
        vikingRoomTexture->createResource(this->RootPath + RESOURSE_PATH + TEXTURE_PATH);
        
        this->modelObject = new object::ModelObject(this->getDevice());
        this->modelObject->setName("Viking Room");
        this->modelObject->RotationAngle(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        helper::loadModel::loadModel(this->RootPath, *this->modelObject->getVertices(), *this->modelObject->getIndices());
        this->modelObject->setResource(vikingRoomTexture);
        this->modelObject->createTexture(VK_FORMAT_R8G8B8A8_SRGB);

        this->modelObject2 = new object::ModelObject(this->getDevice());
        this->modelObject2->setName("Viking Room2");
        this->modelObject2->RotationAngle(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        this->modelObject2->setPosition(glm::vec3(3.0f, 0.0f, 0.0f));
        helper::loadModel::loadModel(this->RootPath, *this->modelObject2->getVertices(), *this->modelObject2->getIndices());
        this->modelObject2->setResource(vikingRoomTexture);
        this->modelObject2->createTexture(VK_FORMAT_R8G8B8A8_SRGB);

        this->modelObjectDescriptor = new vkengine::VK3DModelDescriptor(this->getDevice()->logicaldevice, this->frames);
        this->modelObjectDescriptor->setObject(this->modelObject);
        this->modelObjectDescriptor->setObject(this->modelObject2);

        this->skyMapModelDescriptor = new vkengine::VKSkyMapModelDescriptor(this->getDevice()->logicaldevice, this->frames);
        this->skyMapModelDescriptor->setObject(this->cubeSkybox);

        this->createVertexbuffer();
        this->createIndexBuffer();
        this->createUniformBuffers();
        this->createDescriptor();

        this->createGraphicsPipeline();
        this->createGraphicsPipeline2();
        this->createGraphicsPipeline_skymap();

        this->initUI();

        return true;
    }

    void DescriptorCodeUpdateEngine::cleanup()
    {
        if (this->_isInitialized)
        {
            this->vkGUI->cleanup();
            this->cleanupSwapcChain();

            vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKgraphicsPipeline, nullptr);
            vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKgraphicsPipeline2, nullptr);
            vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKSkyMapPipeline, nullptr);

            vkDestroyRenderPass(this->VKdevice->logicaldevice, *this->VKrenderPass.get(), nullptr);

            this->modelObjectDescriptor->destroyDescriptor();
            this->skyMapModelDescriptor->destroyDescriptor();

            vkDestroyDescriptorPool(this->VKdevice->logicaldevice, this->VKdescriptorPool, nullptr);

            this->cubeSkybox->cleanup();
            this->modelObject->cleanup();
            this->modelObject2->cleanup();

            for (size_t i = 0; i < this->frames; i++)
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

    void DescriptorCodeUpdateEngine::drawFrame()
    {
        // �������� �����ϱ� ���� �������� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        _VK_CHECK_RESULT_(vkWaitForFences(this->VKdevice->logicaldevice, 1, &this->getCurrnetFrameData().VkinFlightFences, VK_TRUE, UINT64_MAX));

        // �̹����� �������� ���� ���� ü�ο��� �̹��� �ε����� �����ɴϴ�.
        // �־��� ����ü�ο��� ���� �̹����� ȹ���ϰ�, 
        // ���������� ��������� �潺�� ����Ͽ� ����ȭ�� �����ϴ� Vulkan API�� �Լ��Դϴ�.
        uint32_t imageIndex = 0;
        VulkanEngine::prepareFame(&imageIndex);

        this->modelObject->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), this->worldMatrix,
            this->camera.get());
        this->modelObject2->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), this->worldMatrix,
            this->camera.get());
        this->cubeSkybox->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), this->camera.get());


        // �÷��׸� �缳���մϴ�. -> �������� ������ �÷��׸� �缳���մϴ�.
        vkResetFences(this->VKdevice->logicaldevice, 1, &this->getCurrnetFrameData().VkinFlightFences);

        // �������� �����ϱ� ���� �̹����� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        // ������ ��� ���۸� �ʱ�ȭ�ϰ�, ���������� �÷��׸� ����Ͽ� �ʱ�ȭ ������ ����
        vkResetCommandBuffer(this->VKframeData[this->currentFrame].mainCommandBuffer, 0);

        // �������� �����ϱ� ���� Ŀ�ǵ� ���۸� �缳���մϴ�.
        this->recordCommandBuffer(&this->VKframeData[this->currentFrame], imageIndex);

        // VkSubmitInfo ����ü�� ť�� ������ ��� ���۸� �����մϴ�.
        VKsubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // �������� �����ϱ� ���� ������� �����մϴ�.
        VkSemaphore waitSemaphores[] = { this->VKframeData[this->currentFrame].VkimageavailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VKsubmitInfo.waitSemaphoreCount = 1;
        VKsubmitInfo.pWaitSemaphores = waitSemaphores;
        VKsubmitInfo.pWaitDstStageMask = waitStages;

        // �������� �����ϱ� ���� Ŀ�ǵ� ���۸� �����մϴ�.
        VKsubmitInfo.commandBufferCount = 1;
        VKsubmitInfo.pCommandBuffers = &this->VKframeData[this->currentFrame].mainCommandBuffer;

        // �������� �����ϱ� ���� ������� �����մϴ�.
        VkSemaphore signalSemaphores[] = { this->VKframeData[this->currentFrame].VkrenderFinishedSemaphore };
        VKsubmitInfo.signalSemaphoreCount = 1;
        VKsubmitInfo.pSignalSemaphores = signalSemaphores;

        // �÷��׸� �缳���մϴ�. -> �������� ������ �÷��׸� �缳���մϴ�.
        vkResetFences(this->VKdevice->logicaldevice, 1, &this->VKframeData[this->currentFrame].VkinFlightFences);

        // �������� �����մϴ�.
        _VK_CHECK_RESULT_(vkQueueSubmit(this->VKdevice->graphicsVKQueue, 1, &VKsubmitInfo, this->VKframeData[this->currentFrame].VkinFlightFences));

        // ������ ���� ��, ������Ʈ�� �����մϴ�.
        VulkanEngine::presentFrame(&imageIndex);

        this->currentFrame = (this->currentFrame + 1) % this->frames;
    }

    bool DescriptorCodeUpdateEngine::mainLoop()
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

#ifdef DEBUG_
            //printf("update\n");
#endif // DEBUG_

        }

        vkDeviceWaitIdle(this->VKdevice->logicaldevice);
        state = false;

        return state;
    }

    void DescriptorCodeUpdateEngine::update(float dt)
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

        this->camera->setPerspectiveProjection(glm::radians(fov), aspectRatio, nearP, farP);
        
        glm::quat rotation = glm::angleAxis(glm::radians(90.0f) * dt,
                                            glm::vec3(0.0f, 1.0f, 0.0f));

        glm::quat rotation2 = glm::angleAxis(glm::radians(90.0f) * dt * -1.0f,
                                            glm::vec3(0.0f, 1.0f, 0.0f));

        glm::quat rotation3 =
            glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 rotationMatrix = vkMath::convertQuatToMatrix(rotation);
        glm::mat4 rotationMatrix2 = vkMath::convertQuatToMatrix(rotation2);

        this->worldMatrix = rotationMatrix * glm::mat4(1.0f);

        this->modelObject2->setRotation(rotation3 * rotation2);
        
        this->modelObject->updateMatrix();
        this->modelObject2->updateMatrix();

    }

    bool DescriptorCodeUpdateEngine::init_sync_structures()
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

    void DescriptorCodeUpdateEngine::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
    {
        // Ŀ�ǵ� ���� ����� �����մϴ�.
        VkCommandBufferBeginInfo beginInfo = framedata->commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        _VK_CHECK_RESULT_(vkBeginCommandBuffer(framedata->mainCommandBuffer, &beginInfo));

        // ���� �н��� �����ϱ� ���� Ŭ���� �� ����
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        // ���� �н� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = *this->VKrenderPass.get();
        renderPassInfo.framebuffer = this->VKswapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = this->VKswapChain->getSwapChainExtent();
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
        renderPassInfo.pClearValues = clearValues.data();

        // ���� �н��� �����մϴ�.
        vkCmdBeginRenderPass(framedata->mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(this->VKswapChain->getSwapChainExtent().width);
            viewport.height = static_cast<float>(this->VKswapChain->getSwapChainExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(framedata->mainCommandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = this->VKswapChain->getSwapChainExtent();
            vkCmdSetScissor(framedata->mainCommandBuffer, 0, 1, &scissor);

            this->skyMapModelDescriptor->BindDescriptorSets(framedata->mainCommandBuffer, this->currentFrame, 0);
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKSkyMapPipeline);
            this->cubeSkybox->draw(framedata->mainCommandBuffer, this->currentFrame);

            this->modelObjectDescriptor->BindDescriptorSets(framedata->mainCommandBuffer, this->currentFrame, 0);
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
            this->modelObject->draw(framedata->mainCommandBuffer, this->currentFrame);
            
            this->modelObjectDescriptor->BindDescriptorSets(framedata->mainCommandBuffer, this->currentFrame, 1);
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline2);
            this->modelObject2->draw(framedata->mainCommandBuffer, this->currentFrame);

            this->vkGUI->render();
        }

        vkCmdEndRenderPass(framedata->mainCommandBuffer);

        // Ŀ�ǵ� ���� ����� �����մϴ�.
        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void DescriptorCodeUpdateEngine::createVertexbuffer()
    {
        this->modelObject->createVertexBuffer(*this->modelObject->getVertices());
        this->modelObject2->createVertexBuffer(*this->modelObject2->getVertices());
        this->cubeSkybox->createVertexBuffer(const_cast<std::vector<Vertex>&>(skyboxVertices));
    }

    void DescriptorCodeUpdateEngine::createIndexBuffer()
    {
        this->modelObject->createIndexBuffer(*this->modelObject->getIndices());
        this->modelObject2->createIndexBuffer(*this->modelObject2->getIndices());
        this->cubeSkybox->createIndexBuffer(const_cast<std::vector<uint16_t>&>(skyboxIndices));
    }

    void DescriptorCodeUpdateEngine::createUniformBuffers()
    {
        this->modelObject->createModelViewProjBuffers();
        this->modelObject2->createModelViewProjBuffers();
        this->cubeSkybox->createModelViewProjBuffers();

        this->modelObject->getTexture()->createDescriptorImageInfo();
        this->modelObject->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
        this->modelObject->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

        this->modelObject2->getTexture()->createDescriptorImageInfo();
        this->modelObject2->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
        this->modelObject2->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

        this->cubeSkybox->getTexture()->createDescriptorImageInfo();
        this->cubeSkybox->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
        this->cubeSkybox->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

    }

    void DescriptorCodeUpdateEngine::createDescriptor()
    {
        this->modelObjectDescriptor->createDescriptorSetLayout(true);
        this->modelObjectDescriptor->createDescriptorPool(true);
        this->modelObjectDescriptor->createDescriptorSets(true);
        this->modelObjectDescriptor->updateDescriptorSets();

        this->skyMapModelDescriptor->createDescriptorSetLayout(false);
        this->skyMapModelDescriptor->createDescriptorPool(false);
        this->skyMapModelDescriptor->createDescriptorSets(false);
        this->skyMapModelDescriptor->updateDescriptorSets();
    }

    void DescriptorCodeUpdateEngine::createGraphicsPipeline()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertloadmodel.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragloadmodel2.spv");

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = baseVertshaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = baseFragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        // �׷��� ���������� ���̾ƿ��� �����մϴ�.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // �Է� �����͸� � ���·� ������ ������ �����մϴ�.
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // �ﰢ�� ����Ʈ�� ����
        inputAssembly.primitiveRestartEnable = VK_FALSE; // ������Ƽ�� ����� ��Ȱ��ȭ

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

        // ����Ʈ ����
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewpport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // ������ȭ ���� -> ������ȭ�� �׷��� ������������ �Ϻη� ������ �ȼ��� ��ȯ�ϴ� ���μ����Դϴ�.
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;                   // ���� Ŭ���� ��Ȱ��ȭ
        rasterizer.rasterizerDiscardEnable = VK_FALSE;            // ������ȭ ���� ��Ȱ��ȭ
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;            // �ٰ��� ��带 ä���� ����
        rasterizer.lineWidth = 1.0f;                              // ���� �ʺ� 1.0f�� ����
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;              // �ĸ� ���� ����
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;   // ���� ���� �ݽð� �������� ����
        rasterizer.depthBiasEnable = VK_FALSE;                    // ���� ���̾ ��Ȱ��ȭ
        rasterizer.depthBiasConstantFactor = 0.0f;              // ���� ���̾ ��� ��Ҹ� 0.0f�� ����
        rasterizer.depthBiasClamp = 0.0f;                       // ���� ���̾ Ŭ������ 0.0f�� ����
        rasterizer.depthBiasSlopeFactor = 0.0f;                 // ���� ���̾ ������ ��Ҹ� 0.0f�� ����

        // ���� ���ø� ����
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //
        multisampling.sampleShadingEnable = VK_FALSE;               // ���ø� ���̵� ��Ȱ��ȭ
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // ������ȭ ���� ���� 1��Ʈ�� ����
        multisampling.minSampleShading = 1.0f;                      // �ּ� ���ø��� 1.0f�� ���� -> option
        multisampling.pSampleMask = nullptr;                        // ���� ����ũ�� nullptr�� ���� -> option
        multisampling.alphaToCoverageEnable = VK_FALSE;             // ���� Ŀ������ ��Ȱ��ȭ -> option
        multisampling.alphaToOneEnable = VK_FALSE;                  // ���� �� ��Ȱ��ȭ -> option

        // ���� ���ٽ� �׽�Ʈ ����
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        // �÷� ���� ����
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;     // �÷� ���� ����ũ�� ����
        colorBlendAttachment.blendEnable = VK_FALSE;                        // ������ ��Ȱ��ȭ
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     // �ҽ� �÷� ���� ���͸� ����
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;    // ��� �÷� ���� ���͸� ����
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;                // �÷� ���� ������ ����
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;     // �ҽ� ���� ���� ���͸� ����
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;    // ��� ���� ���� ���͸� ����
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;                // ���� ���� ������ ����

        // �÷� ���� ���� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; // ����ü Ÿ���� ����
        colorBlending.logicOpEnable = VK_FALSE;                                       // �� ������ ��Ȱ��ȭ
        colorBlending.logicOp = VK_LOGIC_OP_COPY;                                     // �� ������ ����
        colorBlending.attachmentCount = 1;                                            // �÷� ���� ÷�� ������ ����
        colorBlending.pAttachments = &colorBlendAttachment;                           // �÷� ���� ÷�� �����͸� ����
        colorBlending.blendConstants[0] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[1] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[2] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[3] = 0.0f;                                       // ���� ����� ����

        // ���̳��� ���� ���� -> ������ȭ ���¸� �������� ������ �� �ֽ��ϴ�.
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        this->modelObjectDescriptor->createPipelineLayout();

        // �׷��� ���������� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
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
        pipelineInfo.layout = this->modelObjectDescriptor->getPipelineLayout();
        pipelineInfo.renderPass = *this->VKrenderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->VKgraphicsPipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);
    }

    void DescriptorCodeUpdateEngine::createGraphicsPipeline2()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertloadmodel.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragloadmodel2.spv");

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = baseVertshaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = baseFragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        // �׷��� ���������� ���̾ƿ��� �����մϴ�.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // �Է� �����͸� � ���·� ������ ������ �����մϴ�.
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // �ﰢ�� ����Ʈ�� ����
        inputAssembly.primitiveRestartEnable = VK_FALSE; // ������Ƽ�� ����� ��Ȱ��ȭ

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

        // ����Ʈ ����
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewpport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // ������ȭ ���� -> ������ȭ�� �׷��� ������������ �Ϻη� ������ �ȼ��� ��ȯ�ϴ� ���μ����Դϴ�.
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;                   // ���� Ŭ���� ��Ȱ��ȭ
        rasterizer.rasterizerDiscardEnable = VK_FALSE;            // ������ȭ ���� ��Ȱ��ȭ
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;            // �ٰ��� ��带 ä���� ����
        rasterizer.lineWidth = 1.0f;                              // ���� �ʺ� 1.0f�� ����
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;              // �ĸ� ���� ����
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;   // ���� ���� �ݽð� �������� ����
        rasterizer.depthBiasEnable = VK_FALSE;                    // ���� ���̾ ��Ȱ��ȭ
        rasterizer.depthBiasConstantFactor = 0.0f;              // ���� ���̾ ��� ��Ҹ� 0.0f�� ����
        rasterizer.depthBiasClamp = 0.0f;                       // ���� ���̾ Ŭ������ 0.0f�� ����
        rasterizer.depthBiasSlopeFactor = 0.0f;                 // ���� ���̾ ������ ��Ҹ� 0.0f�� ����

        // ���� ���ø� ����
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //
        multisampling.sampleShadingEnable = VK_FALSE;               // ���ø� ���̵� ��Ȱ��ȭ
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // ������ȭ ���� ���� 1��Ʈ�� ����
        multisampling.minSampleShading = 1.0f;                      // �ּ� ���ø��� 1.0f�� ���� -> option
        multisampling.pSampleMask = nullptr;                        // ���� ����ũ�� nullptr�� ���� -> option
        multisampling.alphaToCoverageEnable = VK_FALSE;             // ���� Ŀ������ ��Ȱ��ȭ -> option
        multisampling.alphaToOneEnable = VK_FALSE;                  // ���� �� ��Ȱ��ȭ -> option

        // ���� ���ٽ� �׽�Ʈ ����
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        // �÷� ���� ����
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;     // �÷� ���� ����ũ�� ����
        colorBlendAttachment.blendEnable = VK_FALSE;                        // ������ ��Ȱ��ȭ
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     // �ҽ� �÷� ���� ���͸� ����
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;    // ��� �÷� ���� ���͸� ����
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;                // �÷� ���� ������ ����
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;     // �ҽ� ���� ���� ���͸� ����
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;    // ��� ���� ���� ���͸� ����
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;                // ���� ���� ������ ����

        // �÷� ���� ���� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; // ����ü Ÿ���� ����
        colorBlending.logicOpEnable = VK_FALSE;                                       // �� ������ ��Ȱ��ȭ
        colorBlending.logicOp = VK_LOGIC_OP_COPY;                                     // �� ������ ����
        colorBlending.attachmentCount = 1;                                            // �÷� ���� ÷�� ������ ����
        colorBlending.pAttachments = &colorBlendAttachment;                           // �÷� ���� ÷�� �����͸� ����
        colorBlending.blendConstants[0] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[1] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[2] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[3] = 0.0f;                                       // ���� ����� ����

        // ���̳��� ���� ���� -> ������ȭ ���¸� �������� ������ �� �ֽ��ϴ�.
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // �׷��� ���������� ���̾ƿ��� �����մϴ�.
        //VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // ����ü Ÿ���� ����
        //pipelineLayoutInfo.setLayoutCount = 1;                                    // ���̾ƿ� ������ ����
        //pipelineLayoutInfo.pSetLayouts = &this->VKdescriptorSetLayout;            // ���̾ƿ� �����͸� ����
        //pipelineLayoutInfo.pushConstantRangeCount = 0;                            // Ǫ�� ��� ���� ������ ����
        //pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // Ǫ�� ��� ���� �����͸� ����

        //_VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutInfo, nullptr, &this->VKpipelineLayout));
        //this->modelObjectDescriptor->createPipelineLayout();

        // �׷��� ���������� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
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
        pipelineInfo.layout = this->modelObjectDescriptor->getPipelineLayout();
        pipelineInfo.renderPass = *this->VKrenderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->VKgraphicsPipeline2));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);
    }

    void DescriptorCodeUpdateEngine::createGraphicsPipeline_skymap()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertskybox.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragskybox.spv");

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = baseVertshaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = baseFragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        // �׷��� ���������� ���̾ƿ��� �����մϴ�.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // �Է� �����͸� � ���·� ������ ������ �����մϴ�.
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // �ﰢ�� ����Ʈ�� ����
        inputAssembly.primitiveRestartEnable = VK_FALSE; // ������Ƽ�� ����� ��Ȱ��ȭ

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

        // ����Ʈ ����
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewpport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // ������ȭ ���� -> ������ȭ�� �׷��� ������������ �Ϻη� ������ �ȼ��� ��ȯ�ϴ� ���μ����Դϴ�.
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;                   // ���� Ŭ���� ��Ȱ��ȭ
        rasterizer.rasterizerDiscardEnable = VK_FALSE;            // ������ȭ ���� ��Ȱ��ȭ
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;            // �ٰ��� ��带 ä���� ����
        rasterizer.lineWidth = 1.0f;                              // ���� �ʺ� 1.0f�� ����
        rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;             // skybox�� �ո��� ����
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;   // ���� ���� �ݽð� �������� ����
        rasterizer.depthBiasEnable = VK_FALSE;                    // ���� ���̾ ��Ȱ��ȭ
        rasterizer.depthBiasConstantFactor = 0.0f;              // ���� ���̾ ��� ��Ҹ� 0.0f�� ����
        rasterizer.depthBiasClamp = 0.0f;                       // ���� ���̾ Ŭ������ 0.0f�� ����
        rasterizer.depthBiasSlopeFactor = 0.0f;                 // ���� ���̾ ������ ��Ҹ� 0.0f�� ����

        // ���� ���ø� ����
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //
        multisampling.sampleShadingEnable = VK_FALSE;               // ���ø� ���̵� ��Ȱ��ȭ
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // ������ȭ ���� ���� 1��Ʈ�� ����
        multisampling.minSampleShading = 1.0f;                      // �ּ� ���ø��� 1.0f�� ���� -> option
        multisampling.pSampleMask = nullptr;                        // ���� ����ũ�� nullptr�� ���� -> option
        multisampling.alphaToCoverageEnable = VK_FALSE;             // ���� Ŀ������ ��Ȱ��ȭ -> option
        multisampling.alphaToOneEnable = VK_FALSE;                  // ���� �� ��Ȱ��ȭ -> option

        // ���� ���ٽ� �׽�Ʈ ����
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_FALSE; // ���� ���� ��Ȱ��ȭ
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        // �÷� ���� ����
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;     // �÷� ���� ����ũ�� ����
        colorBlendAttachment.blendEnable = VK_FALSE;                        // ������ ��Ȱ��ȭ
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     // �ҽ� �÷� ���� ���͸� ����
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;    // ��� �÷� ���� ���͸� ����
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;                // �÷� ���� ������ ����
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;     // �ҽ� ���� ���� ���͸� ����
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;    // ��� ���� ���� ���͸� ����
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;                // ���� ���� ������ ����

        // �÷� ���� ���� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; // ����ü Ÿ���� ����
        colorBlending.logicOpEnable = VK_FALSE;                                       // �� ������ ��Ȱ��ȭ
        colorBlending.logicOp = VK_LOGIC_OP_COPY;                                     // �� ������ ����
        colorBlending.attachmentCount = 1;                                            // �÷� ���� ÷�� ������ ����
        colorBlending.pAttachments = &colorBlendAttachment;                           // �÷� ���� ÷�� �����͸� ����
        colorBlending.blendConstants[0] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[1] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[2] = 0.0f;                                       // ���� ����� ����
        colorBlending.blendConstants[3] = 0.0f;                                       // ���� ����� ����

        // ���̳��� ���� ���� -> ������ȭ ���¸� �������� ������ �� �ֽ��ϴ�.
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        this->skyMapModelDescriptor->createPipelineLayout();

        // �׷��� ���������� ���� ���� ����ü�� �ʱ�ȭ�մϴ�.
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
        pipelineInfo.layout = this->skyMapModelDescriptor->getPipelineLayout();
        pipelineInfo.renderPass = *this->VKrenderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->VKSkyMapPipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);
    }

    void DescriptorCodeUpdateEngine::cleanupSwapcChain()
    {
        this->VKdepthStencill.cleanup(this->VKdevice->logicaldevice);

        for (auto framebuffers : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffers, nullptr);
        }

        this->VKswapChain->cleanupSwapChain();
    }

    void DescriptorCodeUpdateEngine::initUI()
    {
        this->vkGUI->init(200, 200);
        this->vkGUI->initResources(this->getRenderPass(), this->getDevice()->graphicsVKQueue, this->getRootPath());
    }
}