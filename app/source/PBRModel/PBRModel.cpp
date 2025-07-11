#include "PBRModel.h"

using namespace vkengine::helper;
using namespace vkengine::debug;

using vkengine::object::Camera;

namespace vkengine {
    PBRModelEngine::PBRModelEngine(std::string root_path) : VulkanEngine(root_path) {}

    PBRModelEngine::~PBRModelEngine() {}

    void PBRModelEngine::init()
    {
        VulkanEngine::init();
        this->camera = std::make_shared<vkengine::object::Camera>();
        this->camera->flipY = false;

    }

    bool PBRModelEngine::prepare()
    {
        VulkanEngine::prepare();
        this->init_sync_structures();
        this->vkGUI = new vkengine::vkGUI(this);

        this->skyBox = new object::SkyBox(this->getDevice());
        cString cubemapPath = this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/pisa_cube.ktx";

        TextureResourceKTX* cubemapResource = new TextureResourceKTX();
        if (!cubemapResource->createResource(cubemapPath)) {
            _PRINT_TO_CONSOLE_("Failed to load cubemap texture from %s\n", cubemapPath.c_str());
            return false;
        }
        this->skyBox->setTextureKTX(cubemapResource);
        this->skyBox->createTexture2(VK_FORMAT_R16G16B16A16_SFLOAT); // 큐브맵 텍스처 생성

        this->uboParams.uboParams.lightPos[0] = cVec4(5.0f, 5.0f, 0.0f, 1.0f);
        this->uboParams.uboParams.lightPos[1] = cVec4(-5.0f, 5.0f, 0.0f, 1.0f);
        this->uboParams.uboParams.lightPos[2] = cVec4(0.0f, 5.0f, 5.0f, 1.0f);
        this->uboParams.uboParams.lightPos[3] = cVec4(0.0f, 5.0f, -5.0f, 1.0f);

        this->uboParams.uboParams.exposure = 1.0f; // 노출 값 설정
        this->uboParams.uboParams.gamma = 2.2f; // 감마 값 설정

        this->initUI();

        // material 버퍼 생성
        this->material = MaterialBuffer
        (
            "Titanium",
            1.0f, // Metallic factor
            0.1f, // Roughness factor
            cVec4(0.541931f, 0.496791f, 0.449419f, 1.0f) // Color (Gold)
        );

        // 모델 오브젝트 생성
        this->modelObject = new object::ModelObject(this->getDevice());
        this->modelObject->setName("sphere");

        cString defaltPath = this->RootPath + RESOURSE_PATH + "image.png";

        TextureResourcePNG* resourcePNG = new TextureResourcePNG();

        resourcePNG->createResource(defaltPath); // 텍스쳐 로드

        if (resourcePNG->data == nullptr) {
            _PRINT_TO_CONSOLE_("Failed to load texture from %s\n", defaltPath.c_str());
            return false;
        }

        this->modelObject->setName("Sphere Object");
        this->modelObject->setTexturePNG(resourcePNG);
        this->modelObject->createTexture(VK_FORMAT_R8G8B8A8_SRGB);
#if 0

        cString modelPath = this->RootPath + RESOURSE_PATH + "sphere2.obj";
        helper::loadModel::loadModel2(modelPath, *this->modelObject->getVertices(), *this->modelObject->getIndices());
        
        this->vikingRoomObject = new object::ModelObject(this->getDevice());
        this->vikingRoomObject->setName("Viking Room");
        this->vikingRoomObject->RotationAngle(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        helper::loadModel::loadModel2(this->RootPath + RESOURSE_PATH + MODEL_PATH, *this->vikingRoomObject->getVertices(), *this->vikingRoomObject->getIndices());

        TextureResourcePNG* vikingRoomTexture = new TextureResourcePNG();
        vikingRoomTexture->createResource(this->RootPath + RESOURSE_PATH + TEXTURE_PATH);

        this->vikingRoomObject->setTexturePNG(vikingRoomTexture);
        this->vikingRoomObject->getTexture()->setMipLevels(1);
        this->vikingRoomObject->createTexture(VK_FORMAT_R8G8B8A8_SRGB);

        this->subUniform.subUniform.lightPos = cVec4(5.0f, 5.0f, 0.0f, 1.0f);

        this->selectModelName = this->modelObject->getName();

        this->modelNames.push_back(this->modelObject->getName());
        this->modelNames.push_back(this->vikingRoomObject->getName());
#endif

        this->createVertexbuffer();
        this->createIndexBuffer();
        this->createUniformBuffers();
        this->createDescriptor();

        //this->createGraphicsPipeline();
        this->createGraphicsPipeline_skymap();

        return true;
    }

    void PBRModelEngine::cleanup()
    {
        if (this->_isInitialized)
        {
            this->vkGUI->cleanup();
            this->cleanupSwapcChain();

            // Pipeline clenup

            vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKSkyMapPipeline, nullptr);
            //vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKgraphicsPipeline, nullptr);

            vkDestroyRenderPass(this->VKdevice->logicaldevice, *this->VKrenderPass.get(), nullptr);

            this->skyboxDescriptor2->destroyDescriptor();
            //this->modeltDescriptor2->destroyDescriptor();

            vkDestroyDescriptorPool(this->VKdevice->logicaldevice, this->VKdescriptorPool, nullptr);

            // object 제거
            this->skyBox->cleanup();
            this->uboParams.cleanup();
            /*this->modelObject->cleanup();
            this->vikingRoomObject->cleanup();
            this->material.cleanup();*/

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

    void PBRModelEngine::drawFrame()
    {
        // 렌더링을 시작하기 전에 프레임을 렌더링할 준비가 되었는지 확인합니다.
        _VK_CHECK_RESULT_(vkWaitForFences(this->VKdevice->logicaldevice, 1, &this->getCurrnetFrameData().VkinFlightFences, VK_TRUE, UINT64_MAX));

        // 이미지를 가져오기 위해 스왑 체인에서 이미지 인덱스를 가져옵니다.
        // 주어진 스왑체인에서 다음 이미지를 획득하고, 
        // 선택적으로 세마포어와 펜스를 사용하여 동기화를 관리하는 Vulkan API의 함수입니다.
        uint32_t imageIndex = 0;
        VulkanEngine::prepareFame(&imageIndex);

        this->skyBox->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), this->camera.get());
        memcpy(this->uboParams.mapped, &this->uboParams.uboParams, sizeof(UniformBufferSkymapParams));

        /*this->modelObject->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), cMat4(1.0f), this->camera.get());
        this->vikingRoomObject->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), cMat4(1.0f), this->camera.get());
        
        memcpy(this->material.mapped, &this->material.material, sizeof(cMaterial));
        memcpy(this->subUniform.mapped, &this->subUniform.subUniform, sizeof(subData));*/

        // 플래그를 재설정합니다. -> 렌더링이 끝나면 플래그를 재설정합니다.
        vkResetFences(this->VKdevice->logicaldevice, 1, &this->getCurrnetFrameData().VkinFlightFences);

        // 렌더링을 시작하기 전에 이미지를 렌더링할 준비가 되었는지 확인합니다.
        // 지정된 명령 버퍼를 초기화하고, 선택적으로 플래그를 사용하여 초기화 동작을 제어
        vkResetCommandBuffer(this->VKframeData[this->currentFrame].mainCommandBuffer, 0);

        // 렌더링을 시작하기 전에 커맨드 버퍼를 재설정합니다.
        this->recordCommandBuffer(&this->VKframeData[this->currentFrame], imageIndex);

        // VkSubmitInfo 구조체는 큐에 제출할 명령 버퍼를 지정합니다.
        VKsubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // 렌더링을 시작하기 전에 세마포어를 설정합니다.
        VkSemaphore waitSemaphores[] = { this->VKframeData[this->currentFrame].VkimageavailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VKsubmitInfo.waitSemaphoreCount = 1;
        VKsubmitInfo.pWaitSemaphores = waitSemaphores;
        VKsubmitInfo.pWaitDstStageMask = waitStages;

        // 렌더링을 시작하기 전에 커맨드 버퍼를 설정합니다.
        VKsubmitInfo.commandBufferCount = 1;
        VKsubmitInfo.pCommandBuffers = &this->VKframeData[this->currentFrame].mainCommandBuffer;

        // 렌더링을 시작하기 전에 세마포어를 설정합니다.
        VkSemaphore signalSemaphores[] = { this->VKframeData[this->currentFrame].VkrenderFinishedSemaphore };
        VKsubmitInfo.signalSemaphoreCount = 1;
        VKsubmitInfo.pSignalSemaphores = signalSemaphores;

        // 플래그를 재설정합니다. -> 렌더링이 끝나면 플래그를 재설정합니다.
        vkResetFences(this->VKdevice->logicaldevice, 1, &this->VKframeData[this->currentFrame].VkinFlightFences);

        // 렌더링을 시작합니다.
        _VK_CHECK_RESULT_(vkQueueSubmit(this->VKdevice->graphicsVKQueue, 1, &VKsubmitInfo, this->VKframeData[this->currentFrame].VkinFlightFences));

        // 렌더링 종료 후, 프레젠트를 시작합니다.
        VulkanEngine::presentFrame(&imageIndex);

        this->currentFrame = (this->currentFrame + 1) % this->frames;
    }

    bool PBRModelEngine::mainLoop()
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

            ImGui::Text("Camera direction: (%.4f, %.4f, %.4f)",
                this->getCamera()->getDir().x,
                this->getCamera()->getDir().y,
                this->getCamera()->getDir().z);

            ImGui::SliderFloat("Gamma", &this->uboParams.uboParams.gamma, 0.1f, 5.0f);
            ImGui::SliderFloat("Exposure", &this->uboParams.uboParams.exposure, 0.1f, 5.0f);

            //cMaterial material = this->material.material;

            //ImGui::Text("cMaterial Name: %s", this->material.name.c_str());
            //ImGui::SliderFloat("Metallic Factor", &material.metallic, 0.0f, 1.0f);
            //ImGui::SliderFloat("Roughness Factor", &material.roughness, 0.0f, 1.0f);
            //ImGui::SliderFloat("Color R", &material.r, 0.0f, 1.0f);
            //ImGui::SliderFloat("Color G", &material.g, 0.0f, 1.0f);
            //ImGui::SliderFloat("Color B", &material.b, 0.0f, 1.0f);

            //ImGui::SliderFloat("Light Position X", &this->subUniform.subUniform.lightPos.x, -10.0f, 10.0f);
            //ImGui::SliderFloat("Light Position Y", &this->subUniform.subUniform.lightPos.y, -10.0f, 10.0f);
            //ImGui::SliderFloat("Light Position Z", &this->subUniform.subUniform.lightPos.z, -10.0f, 10.0f);
            //
            //if (ImGui::BeginCombo("Select Model", selectModelName.c_str())) {
            //    for (int i = 0; i < this->modelNames.size(); ++i) {

            //        const bool isSelected = (selectModelName == this->modelNames[i]);

            //        if (ImGui::Selectable(this->modelNames[i].c_str(), isSelected))
            //        {
            //            this->selectModel = i;
            //            selectModelName = this->modelNames[i];
            //        }

            //        // Set the initial focus when opening the combo
            //        // (scrolling + keyboard navigation focus)
            //        if (isSelected) {
            //            ImGui::SetItemDefaultFocus();
            //        }
            //    }
            //    ImGui::EndCombo();
            //}

            //this->material.material = material;
            //this->subUniform.subUniform.camPos = cVec4(this->camera->getPos(), 0.0f);

            //switch (this->selectModel)
            //{
            //case 0: // Sphere
            //    this->subUniform.subUniform.objectPos = cVec4(this->modelObject->getPosition(), 0.0f);
            //    break;
            //case 1: // Viking Room
            //    this->subUniform.subUniform.objectPos = cVec4(this->vikingRoomObject->getPosition(), 0.0f);
            //    break;
            //default:
            //    this->subUniform.subUniform.objectPos = cVec4(this->modelObject->getPosition(), 0.0f);
            //    break;
            //}

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

    void PBRModelEngine::update(float dt)
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

    bool PBRModelEngine::init_sync_structures()
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

    void PBRModelEngine::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
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

        // 렌더 패스를 시작합니다.
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

            //switch (this->selectModel)
            //{
            //case 0: // Sphere
            //    this->modeltDescriptor2->BindDescriptorSets(framedata->mainCommandBuffer, this->currentFrame, 0);
            //    vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
            //    this->modelObject->draw(framedata->mainCommandBuffer, this->currentFrame);
            //    break;
            //case 1: // Viking Room
            //    this->modeltDescriptor2->BindDescriptorSets(framedata->mainCommandBuffer, this->currentFrame, 1);
            //    vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
            //    this->vikingRoomObject->draw(framedata->mainCommandBuffer, this->currentFrame);
            //    break;
            //default:
            //    this->modeltDescriptor2->BindDescriptorSets(framedata->mainCommandBuffer, this->currentFrame, 0);
            //    vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
            //    this->modelObject->draw(framedata->mainCommandBuffer, this->currentFrame);
            //    break;
            //}

            this->vkGUI->render();
        }

        vkCmdEndRenderPass(framedata->mainCommandBuffer);

        // 커맨드 버퍼 기록을 종료합니다.
        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void PBRModelEngine::createVertexbuffer()
    {
        this->skyBox->createVertexBuffer(const_cast<std::vector<Vertex>&>(skyboxVertices));
        this->modelObject->createVertexBuffer(*this->modelObject->getVertices());
        // this->vikingRoomObject->createVertexBuffer(*this->vikingRoomObject->getVertices());
    }

    void PBRModelEngine::createIndexBuffer()
    {
        this->skyBox->createIndexBuffer(const_cast<std::vector<uint16_t>&>(skyboxIndices));
        this->modelObject->createIndexBuffer(*this->modelObject->getIndices());
        //this->vikingRoomObject->createIndexBuffer(*this->vikingRoomObject->getIndices());
    }

    void PBRModelEngine::createUniformBuffers()
    {
        this->skyBox->createModelViewProjBuffers();
        this->skyBox->getTexture()->createDescriptorImageInfo();
        this->skyBox->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
        this->skyBox->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

        this->uboParams.device = this->VKdevice->logicaldevice;
        this->uboParams.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        this->uboParams.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        
        this->uboParams.createBuffer(this->VKdevice->physicalDevice);
        this->uboParams.mapToMeBuffer(static_cast<VkDeviceSize>(this->uboParams.size), 0);
        this->uboParams.createDescriptorBufferInfo();

        this->modelObject->createModelViewProjBuffers();
        this->modelObject->getTexture()->createDescriptorImageInfo();
        this->modelObject->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
        this->modelObject->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

        //this->vikingRoomObject->createModelViewProjBuffers();
        //this->vikingRoomObject->getTexture()->createDescriptorImageInfo();
        //this->vikingRoomObject->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
        //this->vikingRoomObject->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

        // 머티리얼 버퍼 생성
        VkDeviceSize bufferSize = material.size;

        material.device = this->VKdevice->logicaldevice;
        material.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        material.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        material.createBuffer(this->VKdevice->physicalDevice);
        material.mapToMeBuffer(bufferSize, 0);
        material.createDescriptorBufferInfo();

        //bufferSize = sizeof(subData);

        //this->subUniform.device = this->VKdevice->logicaldevice;
        //this->subUniform.size = bufferSize;
        //this->subUniform.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        //this->subUniform.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        //this->subUniform.createBuffer(this->VKdevice->physicalDevice);
        //this->subUniform.mapToMeBuffer(bufferSize, 0);
        //this->subUniform.createDescriptorBufferInfo();

    }

    void PBRModelEngine::createDescriptor()
    {
        // SkyBox 디스크립터 생성
        this->skyboxDescriptor2 = new VKDescriptor2(this->VKdevice->logicaldevice, this->frames);

        // pool create
        std::vector<VkDescriptorPoolSize> skyboxPoolSizes = {
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames), // 카메라
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames), // 텍스쳐
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames) // 머티리얼
        };
        this->skyboxDescriptor2->poolInfo = helper::descriptorPoolCreateInfo(skyboxPoolSizes, this->frames);
        this->skyboxDescriptor2->createDescriptorPool();

        // layout create
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindingsSkyBox = {
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0), // 카메라
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1), // 텍스쳐
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 2) // 머티리얼
        };
        this->skyboxDescriptor2->layoutInfo = helper::descriptorSetLayoutCreateInfo(setLayoutBindingsSkyBox);
        this->skyboxDescriptor2->createDescriptorSetLayout();

        // descriptor set 생성
        std::vector<VkDescriptorSetLayout> layouts(this->skyboxDescriptor2->frames, this->skyboxDescriptor2->VKdescriptorSetLayout);
        this->skyboxDescriptor2->allocInfo = helper::descriptorSetAllocateInfo(
            this->skyboxDescriptor2->VKdescriptorPool,
            *layouts.data(),
            this->frames); // object가 한개다
        this->skyboxDescriptor2->VKdescriptorSets.resize(this->frames);
        this->skyboxDescriptor2->createAllocateDescriptorSets();

        // descriptor set 업데이트
        std::vector<VkWriteDescriptorSet> writeDescriptorSets_skyBox01 = {
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->skyBox->getModelViewProjUniformBuffer(0)->descriptor), // 카메라
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                &this->skyBox->getTexture()->imageInfo), // 텍스쳐
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                2,
                &this->uboParams.descriptor) // 머티리얼

        };
        // descriptor set 업데이트
        std::vector<VkWriteDescriptorSet> writeDescriptorSets_skyBox02 = {
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->skyBox->getModelViewProjUniformBuffer(1)->descriptor), // 카메라
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                &this->skyBox->getTexture()->imageInfo), // 텍스쳐
            helper::writeDescriptorSet(
                this->skyboxDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                2,
                &this->uboParams.descriptor) // 머티리얼
        };

        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice, 
            static_cast<uint32_t>(writeDescriptorSets_skyBox01.size()), 
            writeDescriptorSets_skyBox01.data(), 0, nullptr);
        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice, 
            static_cast<uint32_t>(writeDescriptorSets_skyBox02.size()), 
            writeDescriptorSets_skyBox02.data(), 0, nullptr);


        // modeltDescriptor2
        this->modeltDescriptor2 = new VKDescriptor2(this->VKdevice->logicaldevice, this->frames);

        // pool create
        std::vector<VkDescriptorPoolSize> modelPoolSizes =
        {
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames), // 카메라
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames), // 머티리얼
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->frames), // 서브 uniform
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames), // albedo
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames), // ao
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames), // height
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames), // metallic
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames), // normal
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames) // roughness
        };
        // layout create

        // descriptor set 생성

        // descriptor set 업데이트

    }


    void PBRModelEngine::createGraphicsPipeline()
    {
#if 0
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

        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // 구조체 타입을 설정
        pipelineLayoutInfo.pNext = nullptr;                                       // 다음 구조체 포인터를 설정
        pipelineLayoutInfo.setLayoutCount = 1;                                    // 레이아웃 개수를 설정
        pipelineLayoutInfo.pSetLayouts = &this->modeltDescriptor2->VKdescriptorSetLayout;            // 레이아웃 포인터를 설정
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // 푸시 상수 범위 개수를 설정
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // 푸시 상수 범위 포인터를 설정

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
#endif

    }

    void PBRModelEngine::createGraphicsPipeline_skymap()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertIBLskybox.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragIBLskybox.spv");

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
            VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
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
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // 구조체 타입을 설정
        pipelineLayoutInfo.pNext = nullptr;                                       // 다음 구조체 포인터를 설정
        pipelineLayoutInfo.setLayoutCount = 1;                                    // 레이아웃 개수를 설정
        pipelineLayoutInfo.pSetLayouts = &this->skyboxDescriptor2->VKdescriptorSetLayout;            // 레이아웃 포인터를 설정
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // 푸시 상수 범위 개수를 설정
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // 푸시 상수 범위 포인터를 설정

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

    void PBRModelEngine::initUI()
    {
        this->vkGUI->init(200, 200);
        this->vkGUI->initResources(this->getRenderPass(), this->getDevice()->graphicsVKQueue, this->getRootPath());
    }

    void PBRModelEngine::cleanupSwapcChain()
    {
        this->VKdepthStencill.cleanup(this->VKdevice->logicaldevice);

        for (auto framebuffers : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffers, nullptr);
        }

        this->VKswapChain->cleanupSwapChain();
    }

}