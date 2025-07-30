#include "computerShader.h"

using namespace vkengine::helper;
using namespace vkengine::debug;

using vkengine::object::Camera;

namespace vkengine {
    computerShaderEngine::computerShaderEngine(std::string root_path) : VulkanEngine(root_path) {}

    computerShaderEngine::~computerShaderEngine() {}

    void computerShaderEngine::init()
    {
        VulkanEngine::init();
        this->camera = std::make_shared<vkengine::object::Camera>();
    }

    bool computerShaderEngine::prepare()
    {
        VulkanEngine::prepare();
        this->init_sync_structures();
        // this->vkGUI = new vkengine::vkGUI(this);

        this->createShaderStorageBuffers();
        // this->createVertexbuffer();
        // this->createIndexBuffer();
        // this->createUniformBuffers();

        this->createDescriptor();

        this->createComputerShaderPipeline();
        this->createGraphicsPipeline();

        this->createComputerCommandBuffer();

        return true;
    }

    void computerShaderEngine::cleanup()
    {
        if (this->_isInitialized)
        {
            //this->vkGUI->cleanup();
            this->cleanupSwapcChain();

            // Pipeline clenup

            vkDestroyRenderPass(this->VKdevice->logicaldevice, *this->VKrenderPass.get(), nullptr);

            this->computershaderDrscriptor->destroyDescriptor();

            vkDestroyDescriptorPool(this->VKdevice->logicaldevice, this->VKdescriptorPool, nullptr);

            vkDestroyPipeline(this->VKdevice->logicaldevice, this->computershaderPipeline, nullptr);
            vkDestroyPipeline(this->VKdevice->logicaldevice, this->graphicsPipeline, nullptr);

            // object ����

            for (size_t i = 0; i < 2; i++)
            {
                vkDestroyBuffer(this->VKdevice->logicaldevice, this->shaderStorageBuffers[i], nullptr);
                vkFreeMemory(this->VKdevice->logicaldevice, this->shaderStorageBuffersMemory[i], nullptr);
                
                vkDestroyBuffer(this->VKdevice->logicaldevice, this->uboTimeBuffers[i], nullptr);
                vkFreeMemory(this->VKdevice->logicaldevice, this->uboTimeBuffersMemory[i], nullptr);
            }

            for (size_t i = 0; i < this->frames; i++)
            {
                vkDestroySemaphore(this->VKdevice->logicaldevice, this->VKframeData[i].VkimageavailableSemaphore, nullptr);
                vkDestroySemaphore(this->VKdevice->logicaldevice, this->VKframeData[i].VkrenderFinishedSemaphore, nullptr);
                vkDestroyFence(this->VKdevice->logicaldevice, this->VKframeData[i].VkinFlightFences, nullptr);

                vkDestroySemaphore(this->VKdevice->logicaldevice, this->computeFrameData[i].computeFinishedSemaphores, nullptr);
                vkDestroyFence(this->VKdevice->logicaldevice, this->computeFrameData[i].VkinFlightFences, nullptr);
            }

            vkDestroyPipelineCache(this->VKdevice->logicaldevice, this->VKpipelineCache, nullptr);
            vkDestroyPipelineLayout(this->VKdevice->logicaldevice, this->pipelineLayout, nullptr);

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

    void computerShaderEngine::drawFrame()
    {
        // sumitInfo ����ü�� �ʱ�ȭ�մϴ�.

        this->VKsubmitInfo = {};

        // �������� �����ϱ� ���� �������� �������� �غ� �Ǿ����� Ȯ���մϴ�.
        _VK_CHECK_RESULT_(vkWaitForFences(this->VKdevice->logicaldevice, 1, &this->computeFrameData[this->currentFrame].VkinFlightFences, VK_TRUE, UINT64_MAX));
        {
            vkResetFences(this->VKdevice->logicaldevice, 1, &this->computeFrameData[this->currentFrame].VkinFlightFences);
            
            this->uboTime.deltaTime = this->getCalulateDeltaTime(); // deltaTime ������Ʈ
            printf("\rTime: %f", this->uboTime.deltaTime);
            memcpy(this->uboTimemapped[this->currentFrame], &this->uboTime, sizeof(UniformBufferTime)); // uniform buffer�� ����

            vkResetCommandBuffer(this->computeFrameData[this->currentFrame].mainCommandBuffer, 0);

            // �������� �����ϱ� ���� Ŀ�ǵ� ���۸� �缳���մϴ�.
            // computer shader ����
            this->recordComputerCommandBuffer(&this->computeFrameData[this->currentFrame]);

            // VkSubmitInfo ����ü�� ť�� ������ ��� ���۸� �����մϴ�.
            // �������� �����ϱ� ���� ������� �����մϴ�.
            VKsubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VKsubmitInfo.commandBufferCount = 1;
            VKsubmitInfo.pCommandBuffers = &this->computeFrameData[this->currentFrame].mainCommandBuffer;
            VKsubmitInfo.signalSemaphoreCount = 1;
            VKsubmitInfo.pSignalSemaphores = &this->computeFrameData[this->currentFrame].computeFinishedSemaphores;
        
            // ť�� ����� �����մϴ�. -> �� �Լ��� �׷��� �������� �����ϴ� �� ť�� computer ť�� ���� ������ �Ѵ�.
            _VK_CHECK_RESULT_(vkQueueSubmit(this->VKdevice->graphicsVKQueue, 1, &VKsubmitInfo, this->computeFrameData[this->currentFrame].VkinFlightFences));
        }

        // Graphics Pipeline
        _VK_CHECK_RESULT_(vkWaitForFences(this->VKdevice->logicaldevice, 1, &this->VKframeData[this->currentFrame].VkinFlightFences, VK_TRUE, UINT64_MAX));

        // �̹����� �������� ���� ���� ü�ο��� �̹��� �ε����� �����ɴϴ�.
        // �־��� ����ü�ο��� ���� �̹����� ȹ���ϰ�, 
        // ���������� ��������� �潺�� ����Ͽ� ����ȭ�� �����ϴ� Vulkan API�� �Լ��Դϴ�.
        uint32_t imageIndex = 0;
        VulkanEngine::prepareFame(&imageIndex);
        {

            // �÷��׸� �缳���մϴ�. -> �������� ������ �÷��׸� �缳���մϴ�.
            vkResetFences(this->VKdevice->logicaldevice, 1, &this->VKframeData[this->currentFrame].VkinFlightFences);

            // �������� �����ϱ� ���� �̹����� �������� �غ� �Ǿ����� Ȯ���մϴ�.
            // ������ ��� ���۸� �ʱ�ȭ�ϰ�, ���������� �÷��׸� ����Ͽ� �ʱ�ȭ ������ ����
            vkResetCommandBuffer(this->VKframeData[this->currentFrame].mainCommandBuffer, 0);

            // ������ ������ �߰�
            this->recordCommandBuffer(&this->VKframeData[this->currentFrame], imageIndex);

            // Semaphore�� ����Ͽ� ��ǻƮ ���̴��� �Ϸ�� �Ŀ� �������� �����մϴ�.
            VkSemaphore waitSemaphores[2] = {
                this->VKframeData[this->currentFrame].VkimageavailableSemaphore, // �̹����� ��� ������ ������ ����մϴ�.
                this->computeFrameData[this->currentFrame].computeFinishedSemaphores // ��ǻƮ ���̴��� �Ϸ�� ������ ����մϴ�.
            };
            
            // ����� ���������� ���������� �����մϴ�.
            VkPipelineStageFlags waitStages[2] = {
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // ���� ÷�� ��� �ܰ迡�� ����մϴ�.
                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT // ��ǻƮ ���̴��� �Ϸ�� �Ŀ� �������� �����մϴ�.
            };
            VKsubmitInfo = {};
            VKsubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VKsubmitInfo.waitSemaphoreCount = 2; // ��ǻƮ ���̴��� �Ϸ�� �Ŀ� �������� �����մϴ�.
            VKsubmitInfo.pWaitSemaphores = waitSemaphores; // ����� ������� �����մϴ�.
            VKsubmitInfo.pWaitDstStageMask = waitStages; // ����� ���������� ���������� �����մϴ�.
            VKsubmitInfo.commandBufferCount = 1; // �������� Ŀ�ǵ� ������ ������ �����մϴ�.
            VKsubmitInfo.pCommandBuffers = &this->VKframeData[this->currentFrame].mainCommandBuffer; // �������� Ŀ�ǵ� ���۸� �����մϴ�.
            VKsubmitInfo.signalSemaphoreCount = 1; // �������� �Ϸ�� �Ŀ� ��ȣ�� ���� ���������� ������ �����մϴ�.
            VKsubmitInfo.pSignalSemaphores = &this->VKframeData[this->currentFrame].VkrenderFinishedSemaphore;
        
        } 
        _VK_CHECK_RESULT_(vkQueueSubmit(this->VKdevice->graphicsVKQueue, 1, &VKsubmitInfo, this->VKframeData[this->currentFrame].VkinFlightFences));
        
        // ������ ���� ��, ������Ʈ�� �����մϴ�.
        VulkanEngine::presentFrame(&imageIndex);

        this->currentFrame = (this->currentFrame + 1) % this->frames;
    }

    bool computerShaderEngine::mainLoop()
    {

        while (!glfwWindowShouldClose(this->VKwindow)) {
            glfwPollEvents();
            //float time = this->getProgramRunTime();

            //this->update(time);

            //this->vkGUI->begin();
            //this->vkGUI->update();
            //
            //ImGui::Text("Camera Yaw, Pitch");
            //ImGui::Text("Yaw: %.4f, Pitch: %.4f", this->getCamera()->getYaw(), this->getCamera()->getPitch());
            //
            //bool check = this->getCameraMoveStyle();
            //ImGui::Checkbox("Camera Move Style", &check);
            //this->setCameraMoveStyle(check);
            //
            //float fov = this->getCamera()->getFov();
            //ImGui::Text("Camera Fov: %.4f", fov);
            //
            //
            //this->vkGUI->end();

            drawFrame();

#ifdef DEBUG_
            //printf("update\n");
#endif // DEBUG_

        }

        vkDeviceWaitIdle(this->VKdevice->logicaldevice);
        state = false;

        return state;
    }

    void computerShaderEngine::update(float dt)
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

        /*this->vikingRoomObject->updateMatrix();
        this->modelObject->updateMatrix();*/
    }

    bool computerShaderEngine::init_sync_structures()
    {
        VkSemaphoreCreateInfo semaphoreInfo = helper::semaphoreCreateInfo(0);
        VkFenceCreateInfo fenceInfo = helper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        for (auto& frameData : this->VKframeData)
        {
            _VK_CHECK_RESULT_(vkCreateSemaphore(this->VKdevice->logicaldevice, &semaphoreInfo, nullptr, &frameData.VkimageavailableSemaphore));
            _VK_CHECK_RESULT_(vkCreateSemaphore(this->VKdevice->logicaldevice, &semaphoreInfo, nullptr, &frameData.VkrenderFinishedSemaphore));
        }
        
        for (auto& frameData : this->computeFrameData)
        {
            _VK_CHECK_RESULT_(vkCreateSemaphore(this->VKdevice->logicaldevice, &semaphoreInfo, nullptr, &frameData.computeFinishedSemaphores));
            _VK_CHECK_RESULT_(vkCreateFence(this->VKdevice->logicaldevice, &fenceInfo, nullptr, &frameData.VkinFlightFences));
        }

        return true;
    }

    void computerShaderEngine::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
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

        vkCmdBeginRenderPass(framedata->mainCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);
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

            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(framedata->mainCommandBuffer, 0, 1, &shaderStorageBuffers[currentFrame], offsets);

            vkCmdDraw(framedata->mainCommandBuffer, MAX_PARTICALES, 1, 0, 0);
        }
        vkCmdEndRenderPass(framedata->mainCommandBuffer);

        // Ŀ�ǵ� ���� ����� �����մϴ�.
        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void computerShaderEngine::recordComputerCommandBuffer(ComputerFrameData* framedata)
    {
        // Ŀ�ǵ� ���� ����� �����մϴ�.
        VkCommandBufferBeginInfo beginInfo = framedata->commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        _VK_CHECK_RESULT_(vkBeginCommandBuffer(framedata->mainCommandBuffer, &beginInfo));
        {
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->computershaderPipeline);
            vkCmdBindDescriptorSets(
                framedata->mainCommandBuffer,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                this->computershaderDrscriptor->VKpipelineLayout,
                0, 1,
                &this->computershaderDrscriptor->VKdescriptorSets[this->currentFrame],
                0, 0);
            vkCmdDispatch(framedata->mainCommandBuffer, MAX_PARTICALES / 256, 1, 1); // ��ǻƮ ���̴��� �����մϴ�. (1x1x1 ��ũ �׷� ũ��)
        }
        // Ŀ�ǵ� ���� ����� �����մϴ�.
        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void computerShaderEngine::createComputerCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(
            this->VKdevice->commandPool,
            1, // Ŀ�ǵ� ���� ����
            VK_COMMAND_BUFFER_LEVEL_PRIMARY); // Ŀ�ǵ� ���� ����

        for (auto& frameData : this->computeFrameData)
        {
            _VK_CHECK_RESULT_(vkAllocateCommandBuffers(this->VKdevice->logicaldevice, &allocInfo, &frameData.mainCommandBuffer));
        }
    }

    void computerShaderEngine::createVertexbuffer()
    {
    }

    void computerShaderEngine::createIndexBuffer()
    {
    }

    void computerShaderEngine::createUniformBuffers()
    {
        
    }

    void computerShaderEngine::createShaderStorageBuffers()
    {
        std::default_random_engine rndEngine((unsigned)time(nullptr));
        std::uniform_real_distribution<float> rndDist(0.0f, 50);

        // ��ƼŬ�� uniform buffer�� �����մϴ�.
        for (auto& particle : this->particles)
        {
            cFloat r = 0.25f * sqrt(rndDist(rndEngine));
            cFloat theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
            cFloat x = r * cos(theta) * HEIGHT / WIDTH;
            cFloat y = r * sin(theta);
            cFloat z = 0.0f;
            particle.position = cVec3(x, y, z);
            particle.velocity = cVec3(glm::normalize(cVec2(x, y)) * 0.00025f, 0.0f); // �ӵ��� ����ȭ�� ���ͷ� �����մϴ�.
            particle.color = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
            particle.empty = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // empty�� ������� ������, ��ƼŬ ����ü�� ũ�⸦ ���߱� ���� ����մϴ�.
        }

        VkDeviceSize bufferSize = sizeof(Particle) * MAX_PARTICALES;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        helper::createBuffer(
            this->VKdevice->logicaldevice,
            this->VKdevice->physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void* data;
        vkMapMemory(this->VKdevice->logicaldevice, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, &particles, (size_t)bufferSize);
        vkUnmapMemory(this->VKdevice->logicaldevice, stagingBufferMemory);

        for (size_t i = 0; i < 2; i++)
        {
            helper::createBuffer(
                this->VKdevice->logicaldevice,
                this->VKdevice->physicalDevice,
                bufferSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                this->shaderStorageBuffers[i],
                this->shaderStorageBuffersMemory[i]);

            // MAX_PARTICALES���� ��ƼŬ�� shaderStorageBuffers[i]�� ����
            // �� �����Ӹ��� ��ƼŬ�� ��ġ�� ������Ʈ
            helper::copyBuffer(
                this->VKdevice->logicaldevice,
                this->VKdevice->commandPool,
                this->VKdevice->graphicsVKQueue,
                stagingBuffer,
                this->shaderStorageBuffers[i],
                bufferSize);

        }

        // ������¡ ���۸� �����մϴ�.
        vkDestroyBuffer(this->VKdevice->logicaldevice, stagingBuffer, nullptr);
        vkFreeMemory(this->VKdevice->logicaldevice, stagingBufferMemory, nullptr);

        bufferSize = sizeof(UniformBufferTime);
        for (size_t i = 0; i < 2; i++)
        {
            helper::createBuffer(
                this->VKdevice->logicaldevice,
                this->VKdevice->physicalDevice,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                this->uboTimeBuffers[i],
                this->uboTimeBuffersMemory[i]);

            // uniform buffer�� �����մϴ�.
            _VK_CHECK_RESULT_(vkMapMemory(this->VKdevice->logicaldevice, this->uboTimeBuffersMemory[i], 0, bufferSize, 0, &this->uboTimemapped[i]));
        }
        
    }

    void computerShaderEngine::createDescriptor()
    {
        this->computershaderDrscriptor = new VKDescriptor2(this->VKdevice->logicaldevice, this->frames);

        // pool create
        std::vector<VkDescriptorPoolSize> poolSizes = {
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames), // deltaTime
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, this->frames), // in Particle -> �б� ����
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, this->frames), // out Particle -> ���� ����
        };

        this->computershaderDrscriptor->poolInfo = helper::descriptorPoolCreateInfo(poolSizes, this->frames);
        this->computershaderDrscriptor->createDescriptorPool();

        // layout create
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0), // deltaTime
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1), // in Particle
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2), // out Particle
        };
        this->computershaderDrscriptor->layoutInfo = helper::descriptorSetLayoutCreateInfo(bindings);
        this->computershaderDrscriptor->createDescriptorSetLayout();

        // descriptor set ����
        std::vector<VkDescriptorSetLayout> layouts(this->frames, this->computershaderDrscriptor->VKdescriptorSetLayout);
        this->computershaderDrscriptor->allocInfo = helper::descriptorSetAllocateInfo(
            this->computershaderDrscriptor->VKdescriptorPool,
            *layouts.data(),
            this->frames); // object�� �Ѱ���
        this->computershaderDrscriptor->VKdescriptorSets.resize(this->frames);
        this->computershaderDrscriptor->createAllocateDescriptorSets();

        VkDescriptorBufferInfo uniformTimeinfo1{};
        VkDescriptorBufferInfo uniformTimeinfo2{};
        VkDescriptorBufferInfo particleInput{};
        VkDescriptorBufferInfo particleOuput{};

        particleInput.buffer = this->shaderStorageBuffers[1];
        particleInput.offset = 0;
        particleInput.range = sizeof(Particle) * MAX_PARTICALES; // MAX_PARTICALES���� ��ƼŬ

        particleOuput.buffer = this->shaderStorageBuffers[0];
        particleOuput.offset = 0;
        particleOuput.range = sizeof(Particle) * MAX_PARTICALES; // MAX_PARTICALES���� ��ƼŬ

        uniformTimeinfo1.buffer = this->uboTimeBuffers[0];
        uniformTimeinfo1.offset = 0;
        uniformTimeinfo1.range = sizeof(UniformBufferTime); // deltaTime

        uniformTimeinfo2.buffer = this->uboTimeBuffers[1];
        uniformTimeinfo2.offset = 0;
        uniformTimeinfo2.range = sizeof(UniformBufferTime); // deltaTime

        // descriptor set ������Ʈ

        // 01 : particleInput -> particleOuput ������ ������Ʈ �Ѵ�.
        std::vector<VkWriteDescriptorSet> writeDescriptorSets01 = {
            helper::writeDescriptorSet(
                this->computershaderDrscriptor->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &uniformTimeinfo1), // deltaTime
            helper::writeDescriptorSet(
                this->computershaderDrscriptor->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                1,
                &particleInput), // in Particle
            helper::writeDescriptorSet(
                this->computershaderDrscriptor->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                2,
                &particleOuput), // out Particle
        };

        // 02 : particleOuput -> particleInput ������ ������Ʈ �Ѵ�.
        std::vector<VkWriteDescriptorSet> writeDescriptorSets02 = {
            helper::writeDescriptorSet(
                this->computershaderDrscriptor->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &uniformTimeinfo2), // deltaTime
            helper::writeDescriptorSet(
                this->computershaderDrscriptor->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                1,
                &particleOuput), // in Particle
            helper::writeDescriptorSet(
                this->computershaderDrscriptor->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                2,
                &particleInput), // out Particle
        };

        // descriptor set ������Ʈ
        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice, 
            static_cast<uint32_t>(writeDescriptorSets01.size()),
            writeDescriptorSets01.data(),
            0, nullptr);
        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice,
            static_cast<uint32_t>(writeDescriptorSets02.size()),
            writeDescriptorSets02.data(),
            0, nullptr);

    }

    void computerShaderEngine::initUI()
    {
        this->vkGUI->init(200, 200);
        this->vkGUI->initResources(this->getRenderPass(), this->getDevice()->graphicsVKQueue, this->getRootPath());
    }

    void computerShaderEngine::createComputerShaderPipeline()
    {
        VkShaderModule computerShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/compCSsample.spv");

        VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
        computeShaderStageInfo = helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, computerShaderModule, "main");

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // ����ü Ÿ���� ����
        pipelineLayoutInfo.pNext = nullptr;                                       // ���� ����ü �����͸� ����
        pipelineLayoutInfo.setLayoutCount = 1;                                    // ���̾ƿ� ������ ����
        pipelineLayoutInfo.pSetLayouts = &this->computershaderDrscriptor->VKdescriptorSetLayout;            // ���̾ƿ� �����͸� ����
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // Ǫ�� ��� ���� ������ ����
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // Ǫ�� ��� ���� �����͸� ����

        _VK_CHECK_RESULT_(vkCreatePipelineLayout(
            this->VKdevice->logicaldevice,
            &pipelineLayoutInfo,
            nullptr,
            &this->computershaderDrscriptor->VKpipelineLayout
        ));

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = this->computershaderDrscriptor->VKpipelineLayout;
        pipelineInfo.stage = computeShaderStageInfo;

        _VK_CHECK_RESULT_(vkCreateComputePipelines(this->VKdevice->logicaldevice,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&this->computershaderPipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, computerShaderModule, nullptr);
    }

    void computerShaderEngine::createGraphicsPipeline()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertParticleVS.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragParticleFS.spv");

        VkPipelineShaderStageCreateInfo shaderStages[2] = {
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, baseVertshaderModule, "main"),
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, baseFragShaderModule, "main")
        };

        // vertex input
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = helper::pipelineVertexInputStateCreateInfo(
            bindingDescription, *attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()), 1);
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = helper::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0, VK_FALSE);

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
        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
        VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
        VkPipelineDynamicStateCreateInfo dynamicState = helper::pipelineDynamicStateCreateInfo(dynamicStates);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = helper::pipelineLayoutCreateInfo(nullptr, 0);
        _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutInfo, nullptr, &this->pipelineLayout));

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

        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphicsPipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);
    }

    void computerShaderEngine::cleanupSwapcChain()
    {
        this->VKdepthStencill.cleanup(this->VKdevice->logicaldevice);

        for (auto framebuffers : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffers, nullptr);
        }

        this->VKswapChain->cleanupSwapChain();
    }

}