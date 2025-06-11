#include "PBRbasic.h"

using namespace vkengine::helper;
using namespace vkengine::debug;

using vkengine::object::Camera;

namespace vkengine {
    PBRbasuceEngube::PBRbasuceEngube(std::string root_path) : VulkanEngine(root_path) {}

    PBRbasuceEngube::~PBRbasuceEngube() {}

    void PBRbasuceEngube::init()
    {
        VulkanEngine::init();
        this->camera = std::make_shared<vkengine::object::Camera>();
    }

    bool PBRbasuceEngube::prepare()
    {
        VulkanEngine::prepare();
        this->init_sync_structures();
        this->vkGUI = new vkengine::vkGUI(this);

        this->skyBox = new object::SkyBox(this->getDevice());

        std::vector<std::string> pathCubeArray = {
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/right.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/left.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/top.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/bottom.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/front.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/back.png"
        };
        
        TextureResource* resource = new TextureResource();

        for (auto& path : pathCubeArray)
        {
            resource->createResource(path);

            if (resource->data == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to load texture from %s\n", path.c_str());
                return false;
            }

            this->skyBox->setTexture(resource);
            free(resource->data); // �޸� ����
            resource->data = nullptr; // ������ �ʱ�ȭ
        }
        this->skyBox->createTexture(VK_FORMAT_R8G8B8A8_SRGB);

        // material ���� ����
        this->material = MaterialBuffer
        (
            "Titanium",
            1.0f, // Metallic factor
            0.1f, // Roughness factor
            cVec4(0.541931f, 0.496791f, 0.449419f, 1.0f) // Color (Gold)
        );

        // �� ������Ʈ ����
        this->modelObject = new object::ModelObject(this->getDevice());
        this->modelObject->setName("sphere");

        resource->createResource(pathCubeArray[0]); // �ؽ��� �ε�

        if (resource->data == nullptr) {
            _PRINT_TO_CONSOLE_("Failed to load texture from %s\n", pathCubeArray[0].c_str());
            return false;
        }

        this->modelObject->setTexture(resource);
        this->modelObject->createTexture(VK_FORMAT_R8G8B8A8_SRGB);

        cString modelPath = this->RootPath + RESOURSE_PATH + "sphere2.obj";
        helper::loadModel::loadModel2(modelPath, *this->modelObject->getVertices(), *this->modelObject->getIndices());
        
        this->vikingRoomObject = new object::ModelObject(this->getDevice());
        this->vikingRoomObject->setName("Viking Room");
        this->vikingRoomObject->RotationAngle(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        this->vikingRoomObject->setPosition(glm::vec3(0.0f, 1.5f, 0.0f));
        helper::loadModel::loadModel2(this->RootPath + RESOURSE_PATH + MODEL_PATH, *this->vikingRoomObject->getVertices(), *this->vikingRoomObject->getIndices());

        TextureResource* vikingRoomTexture = new TextureResource();
        vikingRoomTexture->createResource(this->RootPath + RESOURSE_PATH + TEXTURE_PATH);

        this->vikingRoomObject->setTexture(vikingRoomTexture);
        this->vikingRoomObject->getTexture()->setMipLevels(1);
        this->vikingRoomObject->createTexture(VK_FORMAT_R8G8B8A8_SRGB);

        this->subUniform.subUniform.lightPos = cVec4(5.0f, 5.0f, 0.0f, 1.0f);

        this->createVertexbuffer();
        this->createIndexBuffer();
        this->createUniformBuffers();
        this->createDescriptor();

        this->createGraphicsPipeline();
        this->createGraphicsPipeline_skymap();

        this->initUI();

        return true;
    }

    void PBRbasuceEngube::cleanup()
    {
        if (this->_isInitialized)
        {
            this->vkGUI->cleanup();
            this->cleanupSwapcChain();

            // Pipeline clenup

            vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKSkyMapPipeline, nullptr);
            vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKgraphicsPipeline, nullptr);

            vkDestroyRenderPass(this->VKdevice->logicaldevice, *this->VKrenderPass.get(), nullptr);

            this->skyboxDescriptor2->destroyDescriptor();
            this->modeltDescriptor2->destroyDescriptor();

            vkDestroyDescriptorPool(this->VKdevice->logicaldevice, this->VKdescriptorPool, nullptr);

            // object ����
            this->skyBox->cleanup();
            this->modelObject->cleanup();
            this->vikingRoomObject->cleanup();
            this->material.cleanup();
            this->subUniform.cleanup();

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

    void PBRbasuceEngube::drawFrame()
    {
        // �������� �����ϱ� ���� �������� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        _VK_CHECK_RESULT_(vkWaitForFences(this->VKdevice->logicaldevice, 1, &this->getCurrnetFrameData().VkinFlightFences, VK_TRUE, UINT64_MAX));

        // �̹����� �������� ���� ���� ü�ο��� �̹��� �ε����� �����ɴϴ�.
        // �־��� ����ü�ο��� ���� �̹����� ȹ���ϰ�, 
        // ���������� ��������� �潺�� ����Ͽ� ����ȭ�� �����ϴ� Vulkan API�� �Լ��Դϴ�.
        uint32_t imageIndex = 0;
        VulkanEngine::prepareFame(&imageIndex);

        this->skyBox->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), this->camera.get());
        this->modelObject->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), cMat4(1.0f), this->camera.get());
        this->vikingRoomObject->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), cMat4(1.0f), this->camera.get());
        
        memcpy(this->material.mapped, &this->material.material, sizeof(Material));
        memcpy(this->subUniform.mapped, &this->subUniform.subUniform, sizeof(subData));

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

    bool PBRbasuceEngube::mainLoop()
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

            Material material = this->material.material;

            ImGui::Text("Material Name: %s", this->material.name.c_str());
            ImGui::SliderFloat("Metallic Factor", &material.metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness Factor", &material.roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Color R", &material.r, 0.0f, 1.0f);
            ImGui::SliderFloat("Color G", &material.g, 0.0f, 1.0f);
            ImGui::SliderFloat("Color B", &material.b, 0.0f, 1.0f);

            ImGui::SliderFloat("Light Position X", &this->subUniform.subUniform.lightPos.x, -10.0f, 10.0f);
            ImGui::SliderFloat("Light Position Y", &this->subUniform.subUniform.lightPos.y, -10.0f, 10.0f);
            ImGui::SliderFloat("Light Position Z", &this->subUniform.subUniform.lightPos.z, -10.0f, 10.0f);

            this->material.material = material;

            this->subUniform.subUniform.camPos = cVec4(this->camera->getPos(), 0.0f);
            this->subUniform.subUniform.objectPos = cVec4(this->modelObject->getPosition(), 0.0f);

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

    void PBRbasuceEngube::update(float dt)
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

        this->vikingRoomObject->updateMatrix();
        this->modelObject->updateMatrix();
    }

    bool PBRbasuceEngube::init_sync_structures()
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

    void PBRbasuceEngube::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
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

            this->skyboxDescriptor2->BindDescriptorSets(framedata->mainCommandBuffer, this->currentFrame, 0);
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKSkyMapPipeline);
            this->skyBox->draw(framedata->mainCommandBuffer, this->currentFrame);

            this->modeltDescriptor2->BindDescriptorSets(framedata->mainCommandBuffer, this->currentFrame, 0);
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
            this->modelObject->draw(framedata->mainCommandBuffer, this->currentFrame);

            /*this->modeltDescriptor2->BindDescriptorSets(framedata->mainCommandBuffer, this->currentFrame, 1);
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
            this->vikingRoomObject->draw(framedata->mainCommandBuffer, this->currentFrame);*/


            this->vkGUI->render();
        }

        vkCmdEndRenderPass(framedata->mainCommandBuffer);

        // Ŀ�ǵ� ���� ����� �����մϴ�.
        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void PBRbasuceEngube::createVertexbuffer()
    {
        this->skyBox->createVertexBuffer(const_cast<std::vector<Vertex>&>(skyboxVertices));
        this->modelObject->createVertexBuffer(*this->modelObject->getVertices());
        this->vikingRoomObject->createVertexBuffer(*this->vikingRoomObject->getVertices());
    }

    void PBRbasuceEngube::createIndexBuffer()
    {
        this->skyBox->createIndexBuffer(const_cast<std::vector<uint16_t>&>(skyboxIndices));
        this->modelObject->createIndexBuffer(*this->modelObject->getIndices());
        this->vikingRoomObject->createIndexBuffer(*this->vikingRoomObject->getIndices());
    }

    void PBRbasuceEngube::createUniformBuffers()
    {
        this->skyBox->createModelViewProjBuffers();
        this->skyBox->getTexture()->createDescriptorImageInfo();
        this->skyBox->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
        this->skyBox->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

        this->modelObject->createModelViewProjBuffers();
        this->modelObject->getTexture()->createDescriptorImageInfo();
        this->modelObject->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
        this->modelObject->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

        this->vikingRoomObject->createModelViewProjBuffers();
        this->vikingRoomObject->getTexture()->createDescriptorImageInfo();
        this->vikingRoomObject->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
        this->vikingRoomObject->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

        // ��Ƽ���� ���� ����
        VkDeviceSize bufferSize = material.size;

        material.device = this->VKdevice->logicaldevice;
        material.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        material.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        material.createBuffer(this->VKdevice->physicalDevice);
        material.mapToMeBuffer(bufferSize, 0);
        material.createDescriptorBufferInfo();

        bufferSize = sizeof(subData);

        this->subUniform.device = this->VKdevice->logicaldevice;
        this->subUniform.size = bufferSize;
        this->subUniform.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        this->subUniform.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        this->subUniform.createBuffer(this->VKdevice->physicalDevice);
        this->subUniform.mapToMeBuffer(bufferSize, 0);
        this->subUniform.createDescriptorBufferInfo();

    }

    void PBRbasuceEngube::createDescriptor()
    {

        // �� ������Ʈ ��ũ���� ����
        this->modeltDescriptor2 = new VKDescriptor2(this->VKdevice->logicaldevice, this->frames);

        // pool create
        std::vector<VkDescriptorPoolSize> poolSizes = {
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames * 2), // ī�޶�
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames * 2), // �ؽ���
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames* 2), // ��Ƽ����
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames* 2) // ���� ������
        };
        // cube�� �ֱ⿡ 1�� ����
        // virtual object�� 2���̱⿡ frames * 2�� ����
        this->modeltDescriptor2->poolInfo = helper::descriptorPoolCreateInfo(poolSizes, this->frames * 2);
        this->modeltDescriptor2->createDescriptorPool();

        // layout create
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0), // ī�޶�
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1), // �ؽ���
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 2), // ��Ƽ����
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 3) // subUniform
        };
        this->modeltDescriptor2->layoutInfo = helper::descriptorSetLayoutCreateInfo(setLayoutBindings);
        this->modeltDescriptor2->createDescriptorSetLayout();

        // descriptor set ����
        std::vector<VkDescriptorSetLayout> layouts2(this->modeltDescriptor2->frames * 2, this->modeltDescriptor2->VKdescriptorSetLayout);
        this->modeltDescriptor2->allocInfo = helper::descriptorSetAllocateInfo(
            this->modeltDescriptor2->VKdescriptorPool,
            *layouts2.data(),
            this->frames * 2); // object�� �Ѱ��� -> virtual object�� 2���̱⿡ frames * 2�� ����

        this->modeltDescriptor2->VKdescriptorSets.resize(this->frames * 2); // object 2��
        this->modeltDescriptor2->createAllocateDescriptorSets();

        // descriptor set ������Ʈ
        std::vector<VkWriteDescriptorSet> writeDescriptorSets_PBR_object01 = {
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->modelObject->getModelViewProjUniformBuffer(0)->descriptor), // ī�޶�
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                &this->modelObject->getTexture()->getImageInfo()), // �ؽ���
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                2,
                &this->material.descriptor), // ��Ƽ����
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                3,
                &this->subUniform.descriptor
                )
        };
        std::vector<VkWriteDescriptorSet> writeDescriptorSets_PBR_object02 = {
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->modelObject->getModelViewProjUniformBuffer(1)->descriptor), // ī�޶�
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                &this->modelObject->getTexture()->getImageInfo()), // �ؽ���
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                2,
                &this->material.descriptor), // ��Ƽ����
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                3,
                &this->subUniform.descriptor
                )
        };
        std::vector<VkWriteDescriptorSet> writeDescriptorSets_PBR_object03 = {
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[2],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->vikingRoomObject->getModelViewProjUniformBuffer(0)->descriptor), // ī�޶�
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[2],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                &this->vikingRoomObject->getTexture()->getImageInfo()), // �ؽ���
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[2],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                2,
                &this->material.descriptor), // ��Ƽ����
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[2],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                3,
                &this->subUniform.descriptor
                )
        };
        std::vector<VkWriteDescriptorSet> writeDescriptorSets_PBR_object04 = {
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[3],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->vikingRoomObject->getModelViewProjUniformBuffer(1)->descriptor), // ī�޶�
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[3],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                &this->vikingRoomObject->getTexture()->getImageInfo()), // �ؽ���
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[3],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                2,
                &this->material.descriptor), // ��Ƽ����
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[3],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                3,
                &this->subUniform.descriptor
                )
        };


        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice,
            static_cast<uint32_t>(writeDescriptorSets_PBR_object01.size()),
            writeDescriptorSets_PBR_object01.data(), 0, nullptr);
        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice,
            static_cast<uint32_t>(writeDescriptorSets_PBR_object02.size()),
            writeDescriptorSets_PBR_object02.data(), 0, nullptr);
        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice,
            static_cast<uint32_t>(writeDescriptorSets_PBR_object03.size()),
            writeDescriptorSets_PBR_object03.data(), 0, nullptr);
        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice,
            static_cast<uint32_t>(writeDescriptorSets_PBR_object04.size()),
            writeDescriptorSets_PBR_object04.data(), 0, nullptr);

        // SkyBox ��ũ���� ����
        this->skyboxDescriptor2 = new VKDescriptor2(this->VKdevice->logicaldevice, this->frames);

        // pool create
        std::vector<VkDescriptorPoolSize> skyboxPoolSizes = {
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames), // ī�޶�
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames) // �ؽ���
        };
        this->skyboxDescriptor2->poolInfo = helper::descriptorPoolCreateInfo(skyboxPoolSizes, this->frames);
        this->skyboxDescriptor2->createDescriptorPool();

        // layout create
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindingsSkyBox = {
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0), // ī�޶�
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) // �ؽ���
        };
        this->skyboxDescriptor2->layoutInfo = helper::descriptorSetLayoutCreateInfo(setLayoutBindingsSkyBox);
        this->skyboxDescriptor2->createDescriptorSetLayout();

        // descriptor set ����
        std::vector<VkDescriptorSetLayout> layouts(this->skyboxDescriptor2->frames, this->skyboxDescriptor2->VKdescriptorSetLayout);
        this->skyboxDescriptor2->allocInfo = helper::descriptorSetAllocateInfo(
            this->skyboxDescriptor2->VKdescriptorPool,
            *layouts.data(),
            this->frames); // object�� �Ѱ���
        this->skyboxDescriptor2->VKdescriptorSets.resize(this->frames);
        this->skyboxDescriptor2->createAllocateDescriptorSets();

        // descriptor set ������Ʈ
        std::vector<VkWriteDescriptorSet> writeDescriptorSets_skyBox01 = {
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->skyBox->getModelViewProjUniformBuffer(0)->descriptor), // ī�޶�
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                &this->skyBox->getTexture()->getImageInfo()) // �ؽ���
        };
        // descriptor set ������Ʈ
        std::vector<VkWriteDescriptorSet> writeDescriptorSets_skyBox02 = {
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->skyBox->getModelViewProjUniformBuffer(1)->descriptor), // ī�޶�
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                &this->skyBox->getTexture()->getImageInfo()) // �ؽ���
        };

        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice, 
            static_cast<uint32_t>(writeDescriptorSets_skyBox01.size()), 
            writeDescriptorSets_skyBox01.data(), 0, nullptr);
        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice, 
            static_cast<uint32_t>(writeDescriptorSets_skyBox02.size()), 
            writeDescriptorSets_skyBox02.data(), 0, nullptr);

    }

    void PBRbasuceEngube::createGraphicsPipeline()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertpbr.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragpbr.spv");

        VkPipelineShaderStageCreateInfo shaderStages[2] = {
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, baseVertshaderModule, "main"),
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, baseFragShaderModule, "main")
        };

        // vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = helper::pipelineVertexInputStateCreateInfo(
            bindingDescription, *attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()), 1);
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = helper::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

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
        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, VK_FALSE);
        VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(
            colorBlendAttachment, VK_FALSE, VK_LOGIC_OP_COPY, nullptr);
        VkPipelineDynamicStateCreateInfo dynamicState = helper::pipelineDynamicStateCreateInfo(dynamicStates);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // ����ü Ÿ���� ����
        pipelineLayoutInfo.pNext = nullptr;                                       // ���� ����ü �����͸� ����
        pipelineLayoutInfo.setLayoutCount = 1;                                    // ���̾ƿ� ������ ����
        pipelineLayoutInfo.pSetLayouts = &this->modeltDescriptor2->VKdescriptorSetLayout;            // ���̾ƿ� �����͸� ����
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // Ǫ�� ��� ���� ������ ����
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // Ǫ�� ��� ���� �����͸� ����

        _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutInfo, nullptr, &this->modeltDescriptor2->VKpipelineLayout));

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
        pipelineInfo.layout = this->modeltDescriptor2->VKpipelineLayout;
        pipelineInfo.renderPass = *this->VKrenderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional


        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->VKgraphicsPipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);

    }

    void PBRbasuceEngube::createGraphicsPipeline_skymap()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertskybox.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragskybox.spv");

        VkPipelineShaderStageCreateInfo shaderStages[2] = {
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, baseVertshaderModule, "main"),
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, baseFragShaderModule, "main")
        };

        // vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = helper::pipelineVertexInputStateCreateInfo(
            bindingDescription, *attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()), 1);
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = helper::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

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
        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, VK_FALSE);
        VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(
            colorBlendAttachment, VK_FALSE, VK_LOGIC_OP_COPY, nullptr);
        VkPipelineDynamicStateCreateInfo dynamicState = helper::pipelineDynamicStateCreateInfo(dynamicStates);
        
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // ����ü Ÿ���� ����
        pipelineLayoutInfo.pNext = nullptr;                                       // ���� ����ü �����͸� ����
        pipelineLayoutInfo.setLayoutCount = 1;                                    // ���̾ƿ� ������ ����
        pipelineLayoutInfo.pSetLayouts = &this->skyboxDescriptor2->VKdescriptorSetLayout;            // ���̾ƿ� �����͸� ����
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // Ǫ�� ��� ���� ������ ����
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // Ǫ�� ��� ���� �����͸� ����

        _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutInfo, nullptr, &this->skyboxDescriptor2->VKpipelineLayout));

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
        pipelineInfo.layout = this->skyboxDescriptor2->VKpipelineLayout;
        pipelineInfo.renderPass = *this->VKrenderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional


        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->VKSkyMapPipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);
    }

    void PBRbasuceEngube::initUI()
    {
        this->vkGUI->init(200, 200);
        this->vkGUI->initResources(this->getRenderPass(), this->getDevice()->graphicsVKQueue, this->getRootPath());
    }

    void PBRbasuceEngube::cleanupSwapcChain()
    {
        this->VKdepthStencill.cleanup(this->VKdevice->logicaldevice);

        for (auto framebuffers : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffers, nullptr);
        }

        this->VKswapChain->cleanupSwapChain();
    }

}