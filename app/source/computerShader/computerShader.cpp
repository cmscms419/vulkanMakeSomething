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

            // object 제거

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
        // sumitInfo 구조체를 초기화합니다.

        this->VKsubmitInfo = {};

        // 렌더링을 시작하기 전에 프레임을 렌더링할 준비가 되었는지 확인합니다.
        _VK_CHECK_RESULT_(vkWaitForFences(this->VKdevice->logicaldevice, 1, &this->computeFrameData[this->currentFrame].VkinFlightFences, VK_TRUE, UINT64_MAX));
        {
            vkResetFences(this->VKdevice->logicaldevice, 1, &this->computeFrameData[this->currentFrame].VkinFlightFences);
            
            this->uboTime.deltaTime = this->getCalulateDeltaTime(); // deltaTime 업데이트
            printf("\rTime: %f", this->uboTime.deltaTime);
            memcpy(this->uboTimemapped[this->currentFrame], &this->uboTime, sizeof(UniformBufferTime)); // uniform buffer에 복사

            vkResetCommandBuffer(this->computeFrameData[this->currentFrame].mainCommandBuffer, 0);

            // 렌더링을 시작하기 전에 커맨드 버퍼를 재설정합니다.
            // computer shader 실행
            this->recordComputerCommandBuffer(&this->computeFrameData[this->currentFrame]);

            // VkSubmitInfo 구조체는 큐에 제출할 명령 버퍼를 지정합니다.
            // 렌더링을 시작하기 전에 세마포어를 설정합니다.
            VKsubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VKsubmitInfo.commandBufferCount = 1;
            VKsubmitInfo.pCommandBuffers = &this->computeFrameData[this->currentFrame].mainCommandBuffer;
            VKsubmitInfo.signalSemaphoreCount = 1;
            VKsubmitInfo.pSignalSemaphores = &this->computeFrameData[this->currentFrame].computeFinishedSemaphores;
        
            // 큐에 명령을 제출합니다. -> 이 함수는 그래픽 랜더링을 시작하는 이 큐는 computer 큐와 같은 역할을 한다.
            _VK_CHECK_RESULT_(vkQueueSubmit(this->VKdevice->graphicsVKQueue, 1, &VKsubmitInfo, this->computeFrameData[this->currentFrame].VkinFlightFences));
        }

        // Graphics Pipeline
        _VK_CHECK_RESULT_(vkWaitForFences(this->VKdevice->logicaldevice, 1, &this->VKframeData[this->currentFrame].VkinFlightFences, VK_TRUE, UINT64_MAX));

        // 이미지를 가져오기 위해 스왑 체인에서 이미지 인덱스를 가져옵니다.
        // 주어진 스왑체인에서 다음 이미지를 획득하고, 
        // 선택적으로 세마포어와 펜스를 사용하여 동기화를 관리하는 Vulkan API의 함수입니다.
        uint32_t imageIndex = 0;
        VulkanEngine::prepareFame(&imageIndex);
        {

            // 플래그를 재설정합니다. -> 렌더링이 끝나면 플래그를 재설정합니다.
            vkResetFences(this->VKdevice->logicaldevice, 1, &this->VKframeData[this->currentFrame].VkinFlightFences);

            // 렌더링을 시작하기 전에 이미지를 렌더링할 준비가 되었는지 확인합니다.
            // 지정된 명령 버퍼를 초기화하고, 선택적으로 플래그를 사용하여 초기화 동작을 제어
            vkResetCommandBuffer(this->VKframeData[this->currentFrame].mainCommandBuffer, 0);

            // 랜더할 명형어 추가
            this->recordCommandBuffer(&this->VKframeData[this->currentFrame], imageIndex);

            // Semaphore를 사용하여 컴퓨트 셰이더가 완료된 후에 렌더링을 시작합니다.
            VkSemaphore waitSemaphores[2] = {
                this->VKframeData[this->currentFrame].VkimageavailableSemaphore, // 이미지가 사용 가능할 때까지 대기합니다.
                this->computeFrameData[this->currentFrame].computeFinishedSemaphores // 컴퓨트 셰이더가 완료될 때까지 대기합니다.
            };
            
            // 대기할 파이프라인 스테이지를 지정합니다.
            VkPipelineStageFlags waitStages[2] = {
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // 색상 첨부 출력 단계에서 대기합니다.
                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT // 컴퓨트 셰이더가 완료된 후에 렌더링을 시작합니다.
            };
            VKsubmitInfo = {};
            VKsubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VKsubmitInfo.waitSemaphoreCount = 2; // 컴퓨트 셰이더가 완료된 후에 렌더링을 시작합니다.
            VKsubmitInfo.pWaitSemaphores = waitSemaphores; // 대기할 세마포어를 지정합니다.
            VKsubmitInfo.pWaitDstStageMask = waitStages; // 대기할 파이프라인 스테이지를 지정합니다.
            VKsubmitInfo.commandBufferCount = 1; // 렌더링할 커맨드 버퍼의 개수를 지정합니다.
            VKsubmitInfo.pCommandBuffers = &this->VKframeData[this->currentFrame].mainCommandBuffer; // 렌더링할 커맨드 버퍼를 지정합니다.
            VKsubmitInfo.signalSemaphoreCount = 1; // 렌더링이 완료된 후에 신호를 보낼 세마포어의 개수를 지정합니다.
            VKsubmitInfo.pSignalSemaphores = &this->VKframeData[this->currentFrame].VkrenderFinishedSemaphore;
        
        } 
        _VK_CHECK_RESULT_(vkQueueSubmit(this->VKdevice->graphicsVKQueue, 1, &VKsubmitInfo, this->VKframeData[this->currentFrame].VkinFlightFences));
        
        // 렌더링 종료 후, 프레젠트를 시작합니다.
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
        // 커맨드 버퍼 기록을 시작합니다.
        VkCommandBufferBeginInfo beginInfo = framedata->commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        _VK_CHECK_RESULT_(vkBeginCommandBuffer(framedata->mainCommandBuffer, &beginInfo));

        // 렌더 패스를 시작하기 위한 클리어 값 설정
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
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

        // 커맨드 버퍼 기록을 종료합니다.
        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void computerShaderEngine::recordComputerCommandBuffer(ComputerFrameData* framedata)
    {
        // 커맨드 버퍼 기록을 시작합니다.
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
            vkCmdDispatch(framedata->mainCommandBuffer, MAX_PARTICALES / 256, 1, 1); // 컴퓨트 셰이더를 실행합니다. (1x1x1 워크 그룹 크기)
        }
        // 커맨드 버퍼 기록을 종료합니다.
        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void computerShaderEngine::createComputerCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(
            this->VKdevice->commandPool,
            1, // 커맨드 버퍼 개수
            VK_COMMAND_BUFFER_LEVEL_PRIMARY); // 커맨드 버퍼 레벨

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

        // 파티클의 uniform buffer를 생성합니다.
        for (auto& particle : this->particles)
        {
            cFloat r = 0.25f * sqrt(rndDist(rndEngine));
            cFloat theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
            cFloat x = r * cos(theta) * HEIGHT / WIDTH;
            cFloat y = r * sin(theta);
            cFloat z = 0.0f;
            particle.position = cVec3(x, y, z);
            particle.velocity = cVec3(glm::normalize(cVec2(x, y)) * 0.00025f, 0.0f); // 속도는 정규화된 벡터로 설정합니다.
            particle.color = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
            particle.empty = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // empty는 사용하지 않지만, 파티클 구조체의 크기를 맞추기 위해 사용합니다.
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

            // MAX_PARTICALES개의 파티클을 shaderStorageBuffers[i]에 복사
            // 각 프레임마다 파티클의 위치를 업데이트
            helper::copyBuffer(
                this->VKdevice->logicaldevice,
                this->VKdevice->commandPool,
                this->VKdevice->graphicsVKQueue,
                stagingBuffer,
                this->shaderStorageBuffers[i],
                bufferSize);

        }

        // 스테이징 버퍼를 정리합니다.
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

            // uniform buffer를 매핑합니다.
            _VK_CHECK_RESULT_(vkMapMemory(this->VKdevice->logicaldevice, this->uboTimeBuffersMemory[i], 0, bufferSize, 0, &this->uboTimemapped[i]));
        }
        
    }

    void computerShaderEngine::createDescriptor()
    {
        this->computershaderDrscriptor = new VKDescriptor2(this->VKdevice->logicaldevice, this->frames);

        // pool create
        std::vector<VkDescriptorPoolSize> poolSizes = {
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames), // deltaTime
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, this->frames), // in Particle -> 읽기 전용
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, this->frames), // out Particle -> 쓰기 전용
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

        // descriptor set 생성
        std::vector<VkDescriptorSetLayout> layouts(this->frames, this->computershaderDrscriptor->VKdescriptorSetLayout);
        this->computershaderDrscriptor->allocInfo = helper::descriptorSetAllocateInfo(
            this->computershaderDrscriptor->VKdescriptorPool,
            *layouts.data(),
            this->frames); // object가 한개다
        this->computershaderDrscriptor->VKdescriptorSets.resize(this->frames);
        this->computershaderDrscriptor->createAllocateDescriptorSets();

        VkDescriptorBufferInfo uniformTimeinfo1{};
        VkDescriptorBufferInfo uniformTimeinfo2{};
        VkDescriptorBufferInfo particleInput{};
        VkDescriptorBufferInfo particleOuput{};

        particleInput.buffer = this->shaderStorageBuffers[1];
        particleInput.offset = 0;
        particleInput.range = sizeof(Particle) * MAX_PARTICALES; // MAX_PARTICALES개의 파티클

        particleOuput.buffer = this->shaderStorageBuffers[0];
        particleOuput.offset = 0;
        particleOuput.range = sizeof(Particle) * MAX_PARTICALES; // MAX_PARTICALES개의 파티클

        uniformTimeinfo1.buffer = this->uboTimeBuffers[0];
        uniformTimeinfo1.offset = 0;
        uniformTimeinfo1.range = sizeof(UniformBufferTime); // deltaTime

        uniformTimeinfo2.buffer = this->uboTimeBuffers[1];
        uniformTimeinfo2.offset = 0;
        uniformTimeinfo2.range = sizeof(UniformBufferTime); // deltaTime

        // descriptor set 업데이트

        // 01 : particleInput -> particleOuput 순서로 업데이트 한다.
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

        // 02 : particleOuput -> particleInput 순서로 업데이트 한다.
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

        // descriptor set 업데이트
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
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // 구조체 타입을 설정
        pipelineLayoutInfo.pNext = nullptr;                                       // 다음 구조체 포인터를 설정
        pipelineLayoutInfo.setLayoutCount = 1;                                    // 레이아웃 개수를 설정
        pipelineLayoutInfo.pSetLayouts = &this->computershaderDrscriptor->VKdescriptorSetLayout;            // 레이아웃 포인터를 설정
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // 푸시 상수 범위 개수를 설정
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // 푸시 상수 범위 포인터를 설정

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