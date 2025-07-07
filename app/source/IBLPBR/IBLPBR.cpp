#include "IBLPBR.h"

using namespace vkengine::helper;
using namespace vkengine::debug;

using vkengine::object::Camera;

namespace vkengine {

    iblpbrEngine::iblpbrEngine(std::string root_path) : VulkanEngine(root_path) {}

    iblpbrEngine::~iblpbrEngine() {}

    void iblpbrEngine::init()
    {
        VulkanEngine::init();
        this->camera = std::make_shared<vkengine::object::Camera>();
    }

    bool iblpbrEngine::prepare()
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

        cString modelPath = this->RootPath + RESOURSE_PATH + "teapot.obj";
        cString modelPathGLTF = this->RootPath + RESOURSE_PATH + "sphere.gltf";
        cString modelPathGLTF2 = this->RootPath + RESOURSE_PATH + "glTF/Suzanne.gltf";
#if 0
        helper::loadModel::loadModel2(modelPath, *this->modelObject->getVertices(), *this->modelObject->getIndices());
#else
        helper::loadModel::loadModelGLTF(modelPathGLTF2, *this->modelObject->getVertices(), *this->modelObject->getIndices());
#endif
        //modelObject->setScale(cVec3(0.1f)); // 모델 크기 조정
        //modelObject->setPosition(cVec3(0.0f, -1.0f, 5.0f)); // 모델 위치 설정
        //modelObject->RotationAngle(180.0f, glm::vec3(1.0f, 0.0f, 0.0f)); // 모델 회전 설정

        cMat4 rotation01 = glm::rotate(glm::mat4(1.0f), glm::radians(-180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        cMat4 rotation02 = glm::rotate(glm::mat4(1.0f), glm::radians(-180.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        cMat4 rotation03 = rotation01 * rotation02;
        modelObject->setMatrix(rotation03); // 모델 회전 설정

        //modelObject->setScale(cVec3(0.01f)); // 모델 크기 조정
        //modelObject->updateMatrix();

#if 0
        
        this->vikingRoomObject = new object::ModelObject(this->getDevice());
        this->vikingRoomObject->setName("Viking Room");
        this->vikingRoomObject->RotationAngle(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));

        TextureResourcePNG* vikingRoomTexture = new TextureResourcePNG();
        vikingRoomTexture->createResource(this->RootPath + RESOURSE_PATH + TEXTURE_PATH);

        this->vikingRoomObject->setTexturePNG(vikingRoomTexture);
        this->vikingRoomObject->getTexture()->setMipLevels(1);
        this->vikingRoomObject->createTexture(VK_FORMAT_R8G8B8A8_SRGB);

        this->selectModelName = this->modelObject->getName();

        this->modelNames.push_back(this->modelObject->getName());
        this->modelNames.push_back(this->vikingRoomObject->getName());

#endif

        normalRanderHelpe = new vkengine::helper::normalRander(this);
        normalRanderHelpe->createNormalObject(this->modelObject);

        this->createVertexbuffer();
        this->createIndexBuffer();
        this->createUniformBuffers();

        this->generateBRDFLUT();
        this->generateIrradianceCube();
        this->generatePrefilteredCube();

        this->createDescriptor();

        this->createGraphicsPipeline();
        this->createGraphicsPipeline_skymap();
        this->normalRanderHelpe->createNormalRanderPipeline();
        
        return true;
    }

    void iblpbrEngine::cleanup()
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

            this->normalRanderHelpe->cleanup();

            // object 제거
            this->skyBox->cleanup();
            this->modelObject->cleanup();
            
            this->material.cleanup();
            this->uboParams.cleanup();
            this->subUniform.cleanup();

            this->brdfLUTTexture->cleanup();
            this->prefilteredCubeTexture->cleanup();
            this->irradianceCubeTexture->cleanup();

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

    void iblpbrEngine::drawFrame()
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
        this->modelObject->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), cMat4(1.0f), this->camera.get());

        if (useNormalRander)
        {
            this->normalRanderHelpe->updateCameraUniformBuffer(
                static_cast<uint32_t>(this->currentFrame), cMat4(1.0f), this->camera.get());
        }

        memcpy(this->uboParams.mapped, &this->uboParams.uboParams, sizeof(UniformBufferSkymapParams));
        memcpy(this->material.mapped, &this->material.material, sizeof(cMaterial));
        memcpy(this->subUniform.mapped, &this->subUniform.subUniform, sizeof(subData));

        /*
        this->vikingRoomObject->updateUniformBuffer(
            static_cast<uint32_t>(this->currentFrame), cMat4(1.0f), this->camera.get());
        */

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

    bool iblpbrEngine::mainLoop()
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

            // Material UI
            cMaterial material = this->material.material;

            ImGui::Text("cMaterial Name: %s", this->material.name.c_str());
            ImGui::SliderFloat("Metallic Factor", &material.metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness Factor", &material.roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Color R", &material.r, 0.0f, 1.0f);
            ImGui::SliderFloat("Color G", &material.g, 0.0f, 1.0f);
            ImGui::SliderFloat("Color B", &material.b, 0.0f, 1.0f);

            this->material.material = material;

            // SubUiform UI
            cFloat boolCheck[4] = {
                0.0f, // useTexture
                0.0f, // brdfLUTTexture
                0.0f, // prefilteredCubeTexture
                0.0f  // irradianceCubeTexture
            };

            cBool checkUseTexture = this->subUniform.subUniform.useTexture ? VK_TRUE : VK_FALSE;
            cBool checkBRDFLUTTexture = this->subUniform.subUniform.brdfLUTTexture ? VK_TRUE : VK_FALSE;
            cBool checkPrefilteredCubeTexture = this->subUniform.subUniform.prefilteredCubeTexture ? VK_TRUE : VK_FALSE;
            cBool checkIrradianceCubeTexture = this->subUniform.subUniform.irradianceCubeTexture ? VK_TRUE : VK_FALSE;

            ImGui::Text("Sub Uniform Data");
            ImGui::SliderFloat("Exposure", &this->uboParams.uboParams.exposure, 0.1f, 10.0f);
            ImGui::SliderFloat("Gamma", &this->uboParams.uboParams.gamma, 0.1f, 10.0f);
            ImGui::Checkbox("Use Texture", &checkUseTexture);
            ImGui::Checkbox("Use BRDF LUT Texture", &checkBRDFLUTTexture);
            ImGui::Checkbox("Use Prefiltered Cube Texture", &checkPrefilteredCubeTexture);
            ImGui::Checkbox("Use Irradiance Cube Texture", &checkIrradianceCubeTexture);
            
            // SubUniform 데이터 업데이트
            this->subUniform.subUniform.camPos = this->camera->getPos();
            this->subUniform.subUniform.exposure = this->uboParams.uboParams.exposure;
            this->subUniform.subUniform.gamma = this->uboParams.uboParams.gamma;
            this->subUniform.subUniform.useTexture = checkUseTexture ? VK_TRUE : VK_FALSE; // 텍스쳐 사용 여부
            this->subUniform.subUniform.brdfLUTTexture = checkBRDFLUTTexture ? VK_TRUE : VK_FALSE; // BRDF LUT 텍스처 사용 여부
            this->subUniform.subUniform.prefilteredCubeTexture = checkPrefilteredCubeTexture ? VK_TRUE : VK_FALSE; // Prefiltered Cube 텍스처 사용 여부
            this->subUniform.subUniform.irradianceCubeTexture = checkIrradianceCubeTexture ? VK_TRUE : VK_FALSE; // Irradiance Cube 텍스처 사용 여부

            // normal vector UI
            ImGui::Checkbox("Use Normal Rander", &this->useNormalRander);
            ImGui::SliderFloat("Normal Vector Range", &this->normalRanderObjectScale, 0.01f, 1.0f);
            this->normalRanderHelpe->setNormalRanderObjectScale(this->normalRanderObjectScale);

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

    void iblpbrEngine::update(float dt)
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

    bool iblpbrEngine::init_sync_structures()
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

    void iblpbrEngine::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
    {
        // 커맨드 버퍼 기록을 시작합니다.
        VkCommandBufferBeginInfo beginInfo = framedata->commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        _VK_CHECK_RESULT_(vkBeginCommandBuffer(framedata->mainCommandBuffer, &beginInfo));

        // 렌더 패스를 시작하기 위한 클리어 값 설정
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
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
            this->skyBox->draw(framedata->mainCommandBuffer);

            this->modeltDescriptor2->BindDescriptorSets(framedata->mainCommandBuffer,this->currentFrame,0);
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
            vkCmdPushConstants(framedata->mainCommandBuffer, this->modeltDescriptor2->VKpipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(cVec3), &this->modelObject->getPosition());
            vkCmdPushConstants(framedata->mainCommandBuffer, this->modeltDescriptor2->VKpipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(cVec3), sizeof(cMaterial), &this->material.material);
            this->modelObject->draw(framedata->mainCommandBuffer);
            
            if (useNormalRander)
            {
                this->normalRanderHelpe->recordNormalRanderCommandBuffer(framedata, this->currentFrame);
            }

            this->vkGUI->render();
        }
        vkCmdEndRenderPass(framedata->mainCommandBuffer);

        // 커맨드 버퍼 기록을 종료합니다.
        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void iblpbrEngine::createVertexbuffer()
    {
        this->skyBox->createVertexBuffer(const_cast<std::vector<Vertex>&>(skyboxVertices));
        this->modelObject->createVertexBuffer(*this->modelObject->getVertices());
        //this->vikingRoomObject->createVertexBuffer(*this->vikingRoomObject->getVertices());
    }

    void iblpbrEngine::createIndexBuffer()
    {
        this->skyBox->createIndexBuffer(const_cast<std::vector<uint16_t>&>(skyboxIndices));
        this->modelObject->createIndexBuffer(*this->modelObject->getIndices());
        //this->vikingRoomObject->createIndexBuffer(*this->vikingRoomObject->getIndices());
    }

    void iblpbrEngine::createUniformBuffers()
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

        // 머티리얼 버퍼 생성
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

    void iblpbrEngine::createDescriptor()
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
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames), // samplerIrradiance
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames), // samplerBRDFLUT
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->frames) // prefilteredMap
        };
        this->modeltDescriptor2->poolInfo = helper::descriptorPoolCreateInfo(modelPoolSizes, this->frames);
        this->modeltDescriptor2->createDescriptorPool();

        // layout create
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindingsModel = {
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0), // 카메라
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1), // 머티리얼
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 2), // 서브 uniform
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3), // samplerIrradiance
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4), // samplerBRDFLUT
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5) // prefilteredMap
        };
        this->modeltDescriptor2->layoutInfo = helper::descriptorSetLayoutCreateInfo(setLayoutBindingsModel);
        this->modeltDescriptor2->createDescriptorSetLayout();

        // descriptor set 생성
        std::vector<VkDescriptorSetLayout> layouts2(this->modeltDescriptor2->frames, this->modeltDescriptor2->VKdescriptorSetLayout);
        this->modeltDescriptor2->allocInfo = helper::descriptorSetAllocateInfo(
            this->modeltDescriptor2->VKdescriptorPool,
            *layouts2.data(),
            this->frames); // object가 한개다
        this->modeltDescriptor2->VKdescriptorSets.resize(this->frames);
        this->modeltDescriptor2->createAllocateDescriptorSets();

        // descriptor set 업데이트
        std::vector<VkWriteDescriptorSet> writeDescirptorSets_Model01 = {
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->modelObject->getModelViewProjUniformBuffer(0)->descriptor), // 카메라
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1,
                &this->material.descriptor), // 머티리얼
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                2,
                &this->subUniform.descriptor), // 서브 uniform
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                3,
                &this->irradianceCubeTexture->imageInfo), // samplerIrradiance
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                4,
                &this->brdfLUTTexture->imageInfo), // samplerBRDFLUT
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                5,
                &this->prefilteredCubeTexture->imageInfo) // prefilteredMap
        };

        std::vector<VkWriteDescriptorSet> writeDescirptorSets_Model02 = {
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                0,
                &this->modelObject->getModelViewProjUniformBuffer(1)->descriptor), // 카메라
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1,
                &this->material.descriptor), // 머티리얼
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                2,
                &this->subUniform.descriptor), // 서브 uniform
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                3,
                &this->irradianceCubeTexture->imageInfo), // samplerIrradiance
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                4,
                &this->brdfLUTTexture->imageInfo), // samplerBRDFLUT
            helper::writeDescriptorSet(
                this->modeltDescriptor2->VKdescriptorSets[1],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                5,
                &this->prefilteredCubeTexture->imageInfo) // prefilteredMap
        };

        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice,
            static_cast<uint32_t>(writeDescirptorSets_Model01.size()),
            writeDescirptorSets_Model01.data(), 0, nullptr);
        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice,
            static_cast<uint32_t>(writeDescirptorSets_Model02.size()),
            writeDescirptorSets_Model02.data(), 0, nullptr);
    }

    void iblpbrEngine::createGraphicsPipeline()
    {
#if 1
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertIBLpbr.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragIBLpbr.spv");

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
        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
        VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
        VkPipelineDynamicStateCreateInfo dynamicState = helper::pipelineDynamicStateCreateInfo(dynamicStates);


        std::vector<VkPushConstantRange> pushConstantRanges = {
            helper::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(cVec3), 0), // 푸시 상수 범위 설정
            helper::pushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(cMaterial), sizeof(glm::vec3))
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = 
            helper::pipelineLayoutCreateInfo(&this->modeltDescriptor2->VKdescriptorSetLayout, 1);

        pipelineLayoutInfo.pushConstantRangeCount = 2;
        pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

        _VK_CHECK_RESULT_(
            vkCreatePipelineLayout(
                this->VKdevice->logicaldevice,
                &pipelineLayoutInfo,
                nullptr,
                &this->modeltDescriptor2->VKpipelineLayout));

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

#else


#endif
    }

    void iblpbrEngine::createGraphicsPipeline_skymap()
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
        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, VK_FALSE);
        VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
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

    void iblpbrEngine::initUI()
    {
        this->vkGUI->init(200, 200);
        this->vkGUI->initResources(this->getRenderPass(), this->getDevice()->graphicsVKQueue, this->getRootPath());
    }

    void iblpbrEngine::cleanupSwapcChain()
    {
        this->VKdepthStencill.cleanup(this->VKdevice->logicaldevice);

        for (auto framebuffers : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffers, nullptr);
        }

        this->VKswapChain->cleanupSwapChain();
    }

    void iblpbrEngine::generateBRDFLUT()
    {
        // BRDF LUT 생성 로직을 여기에 추가합니다.
        // 이 함수는 IBL PBR 엔진에서 BRDF LUT를 생성하는 데 사용됩니다.
        // 예를 들어, 렌더 패스를 설정하고, 셰이더를 실행하여 BRDF LUT를 생성할 수 있습니다.
        // 하지만 귀찮은 관계로 파일을 가져오는 형식으로 씀
#if 1
        
        const VkFormat format = VK_FORMAT_R16G16_SFLOAT;	// R16G16 is supported pretty much everywhere
        const int32_t dim = 512;

        brdfLUTTexture = new Vk2DTexture();
        brdfLUTTexture->setDevice(this->VKdevice.get());
        brdfLUTTexture->VKmipLevels = 1; // mipmap 레벨 설정

        // BRDF LUT 텍스쳐 생성
        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = format;
        imageCI.extent.width = dim;
        imageCI.extent.height = dim;
        imageCI.extent.depth = 1;
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        _VK_CHECK_RESULT_(vkCreateImage(this->VKdevice->logicaldevice, &imageCI, nullptr, &this->brdfLUTTexture->image));

        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        VkMemoryRequirements memReqs;

        vkGetImageMemoryRequirements(this->VKdevice->logicaldevice, this->brdfLUTTexture->image, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = helper::findMemoryType(
            this->VKdevice->physicalDevice,
            memReqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        _VK_CHECK_RESULT_(vkAllocateMemory(this->VKdevice->logicaldevice, &memAllocInfo, nullptr, &this->brdfLUTTexture->imageMemory));
        _VK_CHECK_RESULT_(vkBindImageMemory(this->VKdevice->logicaldevice, this->brdfLUTTexture->image, this->brdfLUTTexture->imageMemory, 0));


        // 이미지 뷰 생성
        brdfLUTTexture->createTextureImageView(format);
        
        // 샘플러 생성
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(brdfLUTTexture->device->physicalDevice, &properties);
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;
        _VK_CHECK_RESULT_(vkCreateSampler(brdfLUTTexture->device->logicaldevice, &samplerInfo, nullptr, &brdfLUTTexture->sampler));

        this->brdfLUTTexture->createDescriptorImageInfo();

        // FB, Att, RP, Pipe, etc.
        VkAttachmentDescription attDesc = {};
        // Color attachment
        attDesc.format = VK_FORMAT_R16G16_SFLOAT;
        attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassCI{};
        renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCI.pNext = nullptr;
        renderPassCI.attachmentCount = 1;
        renderPassCI.pAttachments = &attDesc;
        renderPassCI.subpassCount = 1;
        renderPassCI.pSubpasses = &subpassDescription;
        renderPassCI.dependencyCount = 2;
        renderPassCI.pDependencies = dependencies.data();

        VkRenderPass renderpass;
        _VK_CHECK_RESULT_(vkCreateRenderPass(this->VKdevice->logicaldevice, &renderPassCI, nullptr, &renderpass));

        VkFramebufferCreateInfo framebufferCI{};
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.renderPass = renderpass;
        framebufferCI.attachmentCount = 1;
        framebufferCI.pAttachments = &this->brdfLUTTexture->imageView;
        framebufferCI.width = 512;
        framebufferCI.height = 512;
        framebufferCI.layers = 1;

        VkFramebuffer framebuffer{};
        _VK_CHECK_RESULT_(vkCreateFramebuffer(this->VKdevice->logicaldevice, &framebufferCI, nullptr, &framebuffer));

        VKDescriptor2* brdfDescriptors = nullptr;         // 모델 오브젝트 디스크립터
        brdfDescriptors = new VKDescriptor2(this->VKdevice->logicaldevice, 1);

        // Descriptors
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {};
        brdfDescriptors->layoutInfo = helper::descriptorSetLayoutCreateInfo(setLayoutBindings);
        brdfDescriptors->createDescriptorSetLayout();

        // Descriptor Pool
        std::vector<VkDescriptorPoolSize> poolSizes = {
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        };
        brdfDescriptors->poolInfo = helper::descriptorPoolCreateInfo(poolSizes, 1);
        brdfDescriptors->createDescriptorPool();

        // Descriptor sets
        std::vector<VkDescriptorSetLayout> layouts2(1, brdfDescriptors->VKdescriptorSetLayout);

        brdfDescriptors->allocInfo = helper::descriptorSetAllocateInfo(
            brdfDescriptors->VKdescriptorPool, *layouts2.data(), 1);
        brdfDescriptors->VKdescriptorSets.resize(1);

        brdfDescriptors->createAllocateDescriptorSets();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // 구조체 타입을 설정
        pipelineLayoutInfo.pNext = nullptr;                                       // 다음 구조체 포인터를 설정
        pipelineLayoutInfo.setLayoutCount = 1;                                    // 레이아웃 개수를 설정
        pipelineLayoutInfo.pSetLayouts = &brdfDescriptors->VKdescriptorSetLayout;// 레이아웃 포인터를 설정
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // 푸시 상수 범위 개수를 설정
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // 푸시 상수 범위 포인터를 설정

        _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutInfo, nullptr, &brdfDescriptors->VKpipelineLayout));

        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertgenbrdflut.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fraggenbrdflut.spv");

        VkPipelineShaderStageCreateInfo shaderStages[2] = {
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, baseVertshaderModule, "main"),
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, baseFragShaderModule, "main")
        };

        VkViewport viewpport{};
        viewpport.x = 0.0f;
        viewpport.y = 0.0f;
        viewpport.width = 512;
        viewpport.height = 512;
        viewpport.minDepth = 0.0f;
        viewpport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { 512,512 };

        VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
        pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VkPipelineVertexInputStateCreateInfo emptyInputState = pipelineVertexInputStateCreateInfo;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = helper::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
        VkPipelineViewportStateCreateInfo viewportState = helper::pipelineViewportStateCreateInfo(viewpport, scissor, 1, 1);
        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, VK_FALSE);
        VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
        VkPipelineDynamicStateCreateInfo dynamicState = helper::pipelineDynamicStateCreateInfo(dynamicStates);

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &emptyInputState;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState; // Optional
        pipelineInfo.layout = brdfDescriptors->VKpipelineLayout;
        pipelineInfo.renderPass = renderpass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        VkPipeline pipeline;
        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);

        // render
        VkClearValue clearValues[1];
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderpass;
        renderPassBeginInfo.renderArea.extent.width = 512;
        renderPassBeginInfo.renderArea.extent.height = 512;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.framebuffer = framebuffer;

        VkCommandBuffer cmdBuf{};
#if 0
        cmdBuf = this->VKdevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
#else
        cmdBuf = helper::beginSingleTimeCommands(this->VKdevice->logicaldevice, this->getDevice()->commandPool);
#endif
        vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(512);
        viewport.height = static_cast<float>(512);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor2{};
        scissor2.offset = { 0, 0 };
        scissor2.extent = { 512,512 };

        vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
        vkCmdSetScissor(cmdBuf, 0, 1, &scissor2);
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(cmdBuf, 3, 1, 0, 0);
        vkCmdEndRenderPass(cmdBuf);

#if 0
        this->VKdevice->flushCommandBuffer(cmdBuf, this->VKdevice->graphicsVKQueue);

#else
        helper::endSingleTimeCommands(
            this->getDevice()->logicaldevice,
            this->getDevice()->commandPool,
            this->getDevice()->graphicsVKQueue,
            cmdBuf);
#endif

        vkQueueWaitIdle(this->VKdevice->graphicsVKQueue);
        brdfDescriptors->destroyDescriptor();
        vkDestroyPipeline(this->VKdevice->logicaldevice, pipeline, nullptr);
        vkDestroyRenderPass(this->VKdevice->logicaldevice, renderpass, nullptr);
        vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffer, nullptr);

#endif

    }

    void iblpbrEngine::generatePrefilteredCube()
    {
#if 1
        const VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
        const int32_t dim = 512;
        const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

        this->prefilteredCubeTexture = new VKcubeMap();
        this->prefilteredCubeTexture->setDevice(this->VKdevice.get());
        this->prefilteredCubeTexture->VKmipLevels = numMips;

        // Pre-filtered cube map
        // Image
        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = format;
        imageCI.extent.width = dim;
        imageCI.extent.height = dim;
        imageCI.extent.depth = 1;
        imageCI.mipLevels = numMips;
        imageCI.arrayLayers = 6;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        _VK_CHECK_RESULT_(vkCreateImage(this->VKdevice->logicaldevice, &imageCI, nullptr, &this->prefilteredCubeTexture->image));

        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        VkMemoryRequirements memReqs;

        vkGetImageMemoryRequirements(this->VKdevice->logicaldevice, this->prefilteredCubeTexture->image, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = helper::findMemoryType(
            this->VKdevice->physicalDevice,
            memReqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        _VK_CHECK_RESULT_(vkAllocateMemory(this->VKdevice->logicaldevice, &memAllocInfo, nullptr, &this->prefilteredCubeTexture->imageMemory));
        _VK_CHECK_RESULT_(vkBindImageMemory(this->VKdevice->logicaldevice, this->prefilteredCubeTexture->image, this->prefilteredCubeTexture->imageMemory, 0));

        // Image view
        this->prefilteredCubeTexture->createTextureImageView(format);

        // Sampler
        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.minFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.minLod = 0.0f;
        samplerCI.maxLod = static_cast<float>(numMips);
        samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        _VK_CHECK_RESULT_(vkCreateSampler(this->VKdevice->logicaldevice, &samplerCI, nullptr, &this->prefilteredCubeTexture->sampler));

        prefilteredCubeTexture->createDescriptorImageInfo();

        // FB, Att, RP, Pipe, etc.
        VkAttachmentDescription attDesc = {};
        // Color attachment
        attDesc.format = format;
        attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Renderpass
        VkRenderPassCreateInfo renderPassCI{};
        renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCI.attachmentCount = 1;
        renderPassCI.pAttachments = &attDesc;
        renderPassCI.subpassCount = 1;
        renderPassCI.pSubpasses = &subpassDescription;
        renderPassCI.dependencyCount = 2;
        renderPassCI.pDependencies = dependencies.data();

        VkRenderPass renderpass;
        _VK_CHECK_RESULT_(vkCreateRenderPass(this->VKdevice->logicaldevice, &renderPassCI, nullptr, &renderpass));

        struct {
            VkImage image;
            VkImageView view;
            VkDeviceMemory memory;
            VkFramebuffer framebuffer;
        } offscreen;

        // Offscreen framebuffer
        {
            // Color attachment
            VkImageCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = format;
            imageCreateInfo.extent.width = dim;
            imageCreateInfo.extent.height = dim;
            imageCreateInfo.extent.depth = 1;
            imageCreateInfo.mipLevels = 1;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            _VK_CHECK_RESULT_(vkCreateImage(this->VKdevice->logicaldevice, &imageCreateInfo, nullptr, &offscreen.image));

            VkMemoryAllocateInfo memAllocInfo{};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            VkMemoryRequirements memReqs;

            vkGetImageMemoryRequirements(this->VKdevice->logicaldevice, offscreen.image, &memReqs);
            memAllocInfo.allocationSize = memReqs.size;
            memAllocInfo.memoryTypeIndex = helper::findMemoryType(
                this->VKdevice->physicalDevice,
                memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            _VK_CHECK_RESULT_(vkAllocateMemory(this->VKdevice->logicaldevice, &memAllocInfo, nullptr, &offscreen.memory));
            _VK_CHECK_RESULT_(vkBindImageMemory(this->VKdevice->logicaldevice, offscreen.image, offscreen.memory, 0));

            VkImageViewCreateInfo colorImageView{};
            colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorImageView.format = format;
            colorImageView.flags = 0;
            colorImageView.subresourceRange = {};
            colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorImageView.subresourceRange.baseMipLevel = 0;
            colorImageView.subresourceRange.levelCount = 1;
            colorImageView.subresourceRange.baseArrayLayer = 0;
            colorImageView.subresourceRange.layerCount = 1;
            colorImageView.image = offscreen.image;
            _VK_CHECK_RESULT_(vkCreateImageView(this->VKdevice->logicaldevice, &colorImageView, nullptr, &offscreen.view));

            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = renderpass;
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = &offscreen.view;
            fbufCreateInfo.width = dim;
            fbufCreateInfo.height = dim;
            fbufCreateInfo.layers = 1;
            _VK_CHECK_RESULT_(vkCreateFramebuffer(this->VKdevice->logicaldevice, &fbufCreateInfo, nullptr, &offscreen.framebuffer));

            VkCommandBuffer layoutCmd{};
#if 0
            layoutCmd = this->VKdevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
#else
            layoutCmd = helper::beginSingleTimeCommands(this->VKdevice->logicaldevice, this->getDevice()->commandPool);
#endif

            VkImageSubresourceRange subresourceRange{};
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.layerCount = VK_IMAGE_ASPECT_COLOR_BIT;
#if 0
            helper::transitionImageLayout4(
                layoutCmd,
                offscreen.image,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                subresourceRange);
#else
            helper::transitionImageLayout(
                this->getDevice()->logicaldevice,
                this->getDevice()->commandPool,
                this->getDevice()->graphicsVKQueue,
                offscreen.image,
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                1
            );
#endif

#if 0
            this->VKdevice->flushCommandBuffer(layoutCmd, this->VKdevice->graphicsVKQueue);

#else
            helper::endSingleTimeCommands(
                this->getDevice()->logicaldevice,
                this->getDevice()->commandPool,
                this->getDevice()->graphicsVKQueue,
                layoutCmd);
#endif
        }

        VKDescriptor2* prefilteredCubeDescriptors = nullptr;         // 모델 오브젝트 디스크립터
        prefilteredCubeDescriptors = new VKDescriptor2(this->VKdevice->logicaldevice, 1);

        // Descriptors
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
        };
        prefilteredCubeDescriptors->layoutInfo = helper::descriptorSetLayoutCreateInfo(setLayoutBindings);
        prefilteredCubeDescriptors->createDescriptorSetLayout();

        // Descriptor Pool
        std::vector<VkDescriptorPoolSize> poolSizes = { 
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) 
        };
        prefilteredCubeDescriptors->poolInfo = helper::descriptorPoolCreateInfo(poolSizes, 2);
        prefilteredCubeDescriptors->createDescriptorPool();

        // Descriptor sets
        std::vector<VkDescriptorSetLayout> layouts2(1, prefilteredCubeDescriptors->VKdescriptorSetLayout);
        prefilteredCubeDescriptors->allocInfo = helper::descriptorSetAllocateInfo(
            prefilteredCubeDescriptors->VKdescriptorPool, *layouts2.data(), 1);
        prefilteredCubeDescriptors->VKdescriptorSets.resize(1);
        prefilteredCubeDescriptors->createAllocateDescriptorSets();

        // Descriptor set update
        VkWriteDescriptorSet writeDescriptorSets = helper::writeDescriptorSet(
            prefilteredCubeDescriptors->VKdescriptorSets[0],
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            0,
            &this->skyBox->getTexture()->imageInfo); // Pre-filtered cube map texture

        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice,
            1,
            &writeDescriptorSets, 0, nullptr);

        // Pipeline layout
        struct PushBlock {
            glm::mat4 mvp = cMat4(0.0f);
            float roughness = 0.0f;
            uint32_t numSamples = 32u;
        } pushBlock;

        std::vector<VkPushConstantRange> pushConstantRanges = {
            helper::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushBlock), 0),
        };
        VkPipelineLayoutCreateInfo pipelineLayoutCI = helper::pipelineLayoutCreateInfo(&prefilteredCubeDescriptors->VKdescriptorSetLayout, 1);
        pipelineLayoutCI.pushConstantRangeCount = 1;
        pipelineLayoutCI.pPushConstantRanges = pushConstantRanges.data();
        _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutCI, nullptr, &prefilteredCubeDescriptors->VKpipelineLayout));

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(dim);
        viewport.height = static_cast<float>(dim);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { dim, dim };


        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = helper::pipelineVertexInputStateCreateInfo(
            bindingDescription, *attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()), 1);

        // Pipeline
        VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
        pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VkPipelineVertexInputStateCreateInfo emptyInputState = pipelineVertexInputStateCreateInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = helper::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,0, VK_FALSE);
        VkPipelineViewportStateCreateInfo viewportState = helper::pipelineViewportStateCreateInfo(viewport, scissor, 1, 1);
        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, VK_FALSE);
        VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
        VkPipelineDynamicStateCreateInfo dynamicState = helper::pipelineDynamicStateCreateInfo(dynamicStates);

        VkPipelineShaderStageCreateInfo shaderStages[2] = {};

        VkGraphicsPipelineCreateInfo pipelineCI = helper::pipelineCreateInfo(prefilteredCubeDescriptors->VKpipelineLayout, renderpass);
        pipelineCI.pInputAssemblyState = &inputAssembly;
        pipelineCI.pRasterizationState = &rasterizer;
        pipelineCI.pColorBlendState = &colorBlending;
        pipelineCI.pMultisampleState = &multisampling;
        pipelineCI.pViewportState = &viewportState;
        pipelineCI.pDepthStencilState = &depthStencil;
        pipelineCI.pDynamicState = &dynamicState;
        pipelineCI.stageCount = 2;
        pipelineCI.pStages = shaderStages;
        pipelineCI.renderPass = renderpass;
        pipelineCI.pVertexInputState = &vertexInputInfo;

        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertfiltercube.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragprefilterenvmap.spv");

        shaderStages[0] = helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, baseVertshaderModule, "main");
        shaderStages[1] = helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, baseFragShaderModule, "main");

        VkPipeline pipeline;
        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, nullptr, 1, &pipelineCI, nullptr, &pipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);

        // Render

        VkClearValue clearValues[1];
        clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderpass;
        renderPassBeginInfo.framebuffer = offscreen.framebuffer;
        renderPassBeginInfo.renderArea.extent.width = dim;
        renderPassBeginInfo.renderArea.extent.height = dim;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = clearValues;

        std::vector<glm::mat4> matrices = {
            // POSITIVE_X
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_X
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // POSITIVE_Y
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_Y
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // POSITIVE_Z
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_Z
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };


        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = numMips;
        subresourceRange.layerCount = 6;

        // Change image layout for all cubemap faces to transfer destination
#if 0
        helper::transitionImageLayout4(
            cmdBuf,
            this->prefilteredCubeTexture->image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            subresourceRange);
#else
        helper::transitionImageLayout(
            this->getDevice()->logicaldevice,
            this->getDevice()->commandPool,
            this->getDevice()->graphicsVKQueue,
            this->prefilteredCubeTexture->image,
            VK_FORMAT_UNDEFINED,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            numMips,
            6
        );
#endif

#if 0
        VkCommandBuffer cmdBuf = this->VKdevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
#else
        VkCommandBuffer cmdBuf = helper::beginSingleTimeCommands(
            this->getDevice()->logicaldevice,
            this->getDevice()->commandPool);
#endif

        vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
        vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

        for (uint32_t m = 0; m < numMips; m++) {
            pushBlock.roughness = (float)m / (float)(numMips - 1);
            for (uint32_t f = 0; f < 6; f++) {
                viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
                viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
                vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

                // Render scene from cube face's point of view
                vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                // Update shader push constant block
                pushBlock.mvp = glm::perspective((float)(3.14159265358979323846 / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];

                vkCmdPushConstants(cmdBuf, prefilteredCubeDescriptors->VKpipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);

                vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, prefilteredCubeDescriptors->VKpipelineLayout, 0, 1, &prefilteredCubeDescriptors->VKdescriptorSets[0], 0, NULL);

                this->skyBox->draw(cmdBuf, true);

                vkCmdEndRenderPass(cmdBuf);

#if 0
                helper::transitionImageLayout4(
                    cmdBuf,
                    offscreen.image,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    subresourceRange);
#else
                helper::updateimageLayoutcmd(
                    cmdBuf,
                    offscreen.image,
                    VK_FORMAT_UNDEFINED,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                );
#endif
                // Copy region for transfer from framebuffer to cube face
                VkImageCopy copyRegion = {};

                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = { 0, 0, 0 };

                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = f;
                copyRegion.dstSubresource.mipLevel = m;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = { 0, 0, 0 };

                copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
                copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
                copyRegion.extent.depth = 1;

                vkCmdCopyImage(
                    cmdBuf,
                    offscreen.image,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    this->prefilteredCubeTexture->image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &copyRegion);

                // Transform framebuffer color attachment back
                helper::updateimageLayoutcmd(
                    cmdBuf,
                    offscreen.image,
                    VK_FORMAT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                );
            }
        }

#if 0
        this->VKdevice->flushCommandBuffer(cmdBuf, this->VKdevice->graphicsVKQueue);
#else
        helper::endSingleTimeCommands(
            this->getDevice()->logicaldevice,
            this->getDevice()->commandPool,
            this->getDevice()->graphicsVKQueue,
            cmdBuf);
#endif

        helper::transitionImageLayout(
            this->getDevice()->logicaldevice,
            this->getDevice()->commandPool,
            this->getDevice()->graphicsVKQueue,
            this->prefilteredCubeTexture->image,
            VK_FORMAT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            numMips, 6
        );

        vkDestroyRenderPass(this->VKdevice->logicaldevice, renderpass, nullptr);
        vkDestroyFramebuffer(this->VKdevice->logicaldevice, offscreen.framebuffer, nullptr);
        vkFreeMemory(this->VKdevice->logicaldevice, offscreen.memory, nullptr);
        vkDestroyImageView(this->VKdevice->logicaldevice, offscreen.view, nullptr);
        vkDestroyImage(this->VKdevice->logicaldevice, offscreen.image, nullptr);
        prefilteredCubeDescriptors->destroyDescriptor();
        vkDestroyPipeline(this->VKdevice->logicaldevice, pipeline, nullptr);

#else

#endif
    }

    void iblpbrEngine::generateIrradianceCube()
    {

#if 1
        const VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
        const int32_t dim = 64;
        const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

        this->irradianceCubeTexture = new VKcubeMap();
        this->irradianceCubeTexture->setDevice(this->VKdevice.get());
        this->irradianceCubeTexture->VKmipLevels = numMips;

        // Pre-filtered cube map
        // Image
        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = format;
        imageCI.extent.width = dim;
        imageCI.extent.height = dim;
        imageCI.extent.depth = 1;
        imageCI.mipLevels = numMips;
        imageCI.arrayLayers = 6;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        _VK_CHECK_RESULT_(vkCreateImage(this->VKdevice->logicaldevice, &imageCI, nullptr, &this->irradianceCubeTexture->image));

        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        VkMemoryRequirements memReqs;

        vkGetImageMemoryRequirements(this->VKdevice->logicaldevice, this->irradianceCubeTexture->image, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = helper::findMemoryType(
            this->VKdevice->physicalDevice,
            memReqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        _VK_CHECK_RESULT_(vkAllocateMemory(this->VKdevice->logicaldevice, &memAllocInfo, nullptr, &this->irradianceCubeTexture->imageMemory));
        _VK_CHECK_RESULT_(vkBindImageMemory(this->VKdevice->logicaldevice, this->irradianceCubeTexture->image, this->irradianceCubeTexture->imageMemory, 0));

        // Image view
        this->irradianceCubeTexture->createTextureImageView(format);

        // Sampler
        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.minFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.minLod = 0.0f;
        samplerCI.maxLod = static_cast<float>(numMips);
        samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        _VK_CHECK_RESULT_(vkCreateSampler(this->VKdevice->logicaldevice, &samplerCI, nullptr, &this->irradianceCubeTexture->sampler));

        this->irradianceCubeTexture->createDescriptorImageInfo();

        // FB, Att, RP, Pipe, etc.
        VkAttachmentDescription attDesc = {};
        // Color attachment
        attDesc.format = format;
        attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Renderpass
        VkRenderPassCreateInfo renderPassCI{};
        renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCI.attachmentCount = 1;
        renderPassCI.pAttachments = &attDesc;
        renderPassCI.subpassCount = 1;
        renderPassCI.pSubpasses = &subpassDescription;
        renderPassCI.dependencyCount = 2;
        renderPassCI.pDependencies = dependencies.data();

        VkRenderPass renderpass;
        _VK_CHECK_RESULT_(vkCreateRenderPass(this->VKdevice->logicaldevice, &renderPassCI, nullptr, &renderpass));

        struct {
            VkImage image;
            VkImageView view;
            VkDeviceMemory memory;
            VkFramebuffer framebuffer;
        } offscreen;

        // Offscreen framebuffer
        {
            // Color attachment
            VkImageCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = format;
            imageCreateInfo.extent.width = dim;
            imageCreateInfo.extent.height = dim;
            imageCreateInfo.extent.depth = 1;
            imageCreateInfo.mipLevels = 1;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            _VK_CHECK_RESULT_(vkCreateImage(this->VKdevice->logicaldevice, &imageCreateInfo, nullptr, &offscreen.image));


            VkMemoryAllocateInfo memAllocInfo{};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            VkMemoryRequirements memReqs;

            vkGetImageMemoryRequirements(this->VKdevice->logicaldevice, offscreen.image, &memReqs);
            memAllocInfo.allocationSize = memReqs.size;
            memAllocInfo.memoryTypeIndex = helper::findMemoryType(
                this->VKdevice->physicalDevice,
                memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            _VK_CHECK_RESULT_(vkAllocateMemory(this->VKdevice->logicaldevice, &memAllocInfo, nullptr, &offscreen.memory));
            _VK_CHECK_RESULT_(vkBindImageMemory(this->VKdevice->logicaldevice, offscreen.image, offscreen.memory, 0));

            VkImageViewCreateInfo colorImageView{};
            colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorImageView.format = format;
            colorImageView.flags = 0;
            colorImageView.subresourceRange = {};
            colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorImageView.subresourceRange.baseMipLevel = 0;
            colorImageView.subresourceRange.levelCount = 1;
            colorImageView.subresourceRange.baseArrayLayer = 0;
            colorImageView.subresourceRange.layerCount = 1;
            colorImageView.image = offscreen.image;
            _VK_CHECK_RESULT_(vkCreateImageView(this->VKdevice->logicaldevice, &colorImageView, nullptr, &offscreen.view));

            VkFramebufferCreateInfo fbufCreateInfo{};
            fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbufCreateInfo.renderPass = renderpass;
            fbufCreateInfo.attachmentCount = 1;
            fbufCreateInfo.pAttachments = &offscreen.view;
            fbufCreateInfo.width = dim;
            fbufCreateInfo.height = dim;
            fbufCreateInfo.layers = 1;
            _VK_CHECK_RESULT_(vkCreateFramebuffer(this->VKdevice->logicaldevice, &fbufCreateInfo, nullptr, &offscreen.framebuffer));

#if 0
            VkCommandBuffer layoutCmd{};
            layoutCmd = helper::beginSingleTimeCommands(this->VKdevice->logicaldevice, this->getDevice()->commandPool);

            helper::updateimageLayoutcmd(
                layoutCmd,
                offscreen.image,
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                );

            helper::endSingleTimeCommands(
                this->getDevice()->logicaldevice,
                this->getDevice()->commandPool,
                this->getDevice()->graphicsVKQueue,
                layoutCmd);
#else
            helper::transitionImageLayout(
                this->getDevice()->logicaldevice,
                this->getDevice()->commandPool,
                this->getDevice()->graphicsVKQueue,
                offscreen.image,
                VK_FORMAT_UNDEFINED,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            );
#endif
        }

        VKDescriptor2* irradianceCubeDescriptors = nullptr;         // 모델 오브젝트 디스크립터
        irradianceCubeDescriptors = new VKDescriptor2(this->VKdevice->logicaldevice, 1);

        // Descriptors
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
        };
        irradianceCubeDescriptors->layoutInfo = helper::descriptorSetLayoutCreateInfo(setLayoutBindings);
        irradianceCubeDescriptors->createDescriptorSetLayout();

        // Descriptor Pool
        std::vector<VkDescriptorPoolSize> poolSizes = { 
            helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        };
        irradianceCubeDescriptors->poolInfo = helper::descriptorPoolCreateInfo(poolSizes, 2);
        irradianceCubeDescriptors->createDescriptorPool();

        // Descriptor sets
        std::vector<VkDescriptorSetLayout> layouts2(1, irradianceCubeDescriptors->VKdescriptorSetLayout);
        irradianceCubeDescriptors->allocInfo = helper::descriptorSetAllocateInfo(
            irradianceCubeDescriptors->VKdescriptorPool, *layouts2.data(), 1);
        irradianceCubeDescriptors->VKdescriptorSets.resize(1);
        irradianceCubeDescriptors->createAllocateDescriptorSets();
        
        // descriptor set 업데이트
        std::vector<VkWriteDescriptorSet> writeDescriptorSets01 = {
            helper::writeDescriptorSet(
                irradianceCubeDescriptors->VKdescriptorSets[0],
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                0,
                &this->skyBox->getTexture()->imageInfo) //
        };
        
        vkUpdateDescriptorSets(
            this->VKdevice->logicaldevice,
            static_cast<uint32_t>(writeDescriptorSets01.size()),
            writeDescriptorSets01.data(), 0, nullptr);
        
        // Pipeline layout
        struct PushBlock {
            glm::mat4 mvp = cMat4(0.0f); // Model-view-projection matrix
            // Sampling deltas
            float deltaPhi = (2.0f * float(3.14159265358979323846)) / 180.0f;
            float deltaTheta = (0.5f * float(3.14159265358979323846)) / 64.0f;
        } pushBlock;

        std::vector<VkPushConstantRange> pushConstantRanges = {
            helper::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushBlock), 0),
        };
        VkPipelineLayoutCreateInfo pipelineLayoutCI = helper::pipelineLayoutCreateInfo(&irradianceCubeDescriptors->VKdescriptorSetLayout, 1);
        pipelineLayoutCI.pushConstantRangeCount = 1;
        pipelineLayoutCI.pPushConstantRanges = pushConstantRanges.data();
        _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutCI, nullptr, &irradianceCubeDescriptors->VKpipelineLayout));

        VkViewport viewpport = helper::createViewport(0.0f, 0.0f, static_cast<float>(dim), static_cast<float>(dim), 0.0f, 1.0f);
        VkRect2D scissor = helper::createScissor(0, 0, dim, dim);
        
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = helper::pipelineVertexInputStateCreateInfo(
            bindingDescription, *attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()), 1);

        // Pipeline
        VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
        pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VkPipelineVertexInputStateCreateInfo emptyInputState = pipelineVertexInputStateCreateInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = helper::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
        VkPipelineViewportStateCreateInfo viewportState = helper::pipelineViewportStateCreateInfo(viewpport, scissor);
        VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
        VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
        VkPipelineDynamicStateCreateInfo dynamicState = helper::pipelineDynamicStateCreateInfo(dynamicStates);

        VkPipelineShaderStageCreateInfo shaderStages[2] = {};

        VkGraphicsPipelineCreateInfo pipelineCI = helper::pipelineCreateInfo(irradianceCubeDescriptors->VKpipelineLayout, renderpass);
        pipelineCI.pInputAssemblyState = &inputAssembly;
        pipelineCI.pRasterizationState = &rasterizer;
        pipelineCI.pColorBlendState = &colorBlending;
        pipelineCI.pMultisampleState = &multisampling;
        pipelineCI.pViewportState = &viewportState;
        pipelineCI.pDepthStencilState = &depthStencil;
        pipelineCI.pDynamicState = &dynamicState;
        pipelineCI.stageCount = 2;
        pipelineCI.pStages = shaderStages;
        pipelineCI.renderPass = renderpass;
        pipelineCI.pVertexInputState = &vertexInputInfo;

        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertfiltercube.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragirradiancecube.spv");

        shaderStages[0] = helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, baseVertshaderModule, "main");
        shaderStages[1] = helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, baseFragShaderModule, "main");

        VkPipeline pipeline;
        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, nullptr, 1, &pipelineCI, nullptr, &pipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);

        // Render
        VkClearValue clearValues[1];
        clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderpass;
        renderPassBeginInfo.framebuffer = offscreen.framebuffer;
        renderPassBeginInfo.renderArea.extent.width = dim;
        renderPassBeginInfo.renderArea.extent.height = dim;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = clearValues;

        std::vector<glm::mat4> matrices = {
            // POSITIVE_X
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_X
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // POSITIVE_Y
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_Y
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // POSITIVE_Z
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            // NEGATIVE_Z
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };

        // Change image layout for all cubemap faces to transfer destination
        helper::transitionImageLayout(
            this->getDevice()->logicaldevice,
            this->getDevice()->commandPool,
            this->getDevice()->graphicsVKQueue,
            this->irradianceCubeTexture->image,
            VK_FORMAT_UNDEFINED,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
            numMips, 6
        );

#if 0
        VkCommandBuffer cmdBuf = this->VKdevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
#else
        VkCommandBuffer cmdBuf = helper::beginSingleTimeCommands(
            this->getDevice()->logicaldevice,
            this->getDevice()->commandPool);
#endif

        vkCmdSetViewport(cmdBuf, 0, 1, &viewpport);
        vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

        for (uint32_t m = 0; m < numMips; m++) {
            for (uint32_t f = 0; f < 6; f++) {
                viewpport.width = static_cast<float>(dim * std::pow(0.5f, m));
                viewpport.height = static_cast<float>(dim * std::pow(0.5f, m));
                vkCmdSetViewport(cmdBuf, 0, 1, &viewpport);

                // Render scene from cube face's point of view
                vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                // Update shader push constant block
                pushBlock.mvp = glm::perspective((float)(3.14159265358979323846 / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];

                vkCmdPushConstants(cmdBuf, irradianceCubeDescriptors->VKpipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);

                vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, irradianceCubeDescriptors->VKpipelineLayout, 0, 1, &irradianceCubeDescriptors->VKdescriptorSets[0], 0, NULL);

                this->skyBox->draw(cmdBuf, true);

                vkCmdEndRenderPass(cmdBuf);

                helper::updateimageLayoutcmd(
                    cmdBuf,
                    offscreen.image,
                    VK_FORMAT_UNDEFINED,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                    );

                // Copy region for transfer from framebuffer to cube face
                VkImageCopy copyRegion = {};

                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = { 0, 0, 0 };

                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = f;
                copyRegion.dstSubresource.mipLevel = m;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = { 0, 0, 0 };

                copyRegion.extent.width = static_cast<uint32_t>(viewpport.width);
                copyRegion.extent.height = static_cast<uint32_t>(viewpport.height);
                copyRegion.extent.depth = 1;

                vkCmdCopyImage(
                    cmdBuf,
                    offscreen.image,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    this->irradianceCubeTexture->image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &copyRegion);

                // Transform framebuffer color attachment back
                helper::updateimageLayoutcmd(
                    cmdBuf,
                    offscreen.image,
                    VK_FORMAT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                );
            }
        }

        helper::transitionImageLayout(
            this->getDevice()->logicaldevice,
            this->getDevice()->commandPool,
            this->getDevice()->graphicsVKQueue,
            this->irradianceCubeTexture->image,
            VK_FORMAT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

#if 0
        this->VKdevice->flushCommandBuffer(cmdBuf, this->VKdevice->graphicsVKQueue);
#else
        helper::endSingleTimeCommands(
            this->getDevice()->logicaldevice,
            this->getDevice()->commandPool,
            this->getDevice()->graphicsVKQueue,
            cmdBuf);
#endif

        vkDestroyRenderPass(this->VKdevice->logicaldevice, renderpass, nullptr);
        vkDestroyFramebuffer(this->VKdevice->logicaldevice, offscreen.framebuffer, nullptr);
        vkFreeMemory(this->VKdevice->logicaldevice, offscreen.memory, nullptr);
        vkDestroyImageView(this->VKdevice->logicaldevice, offscreen.view, nullptr);
        vkDestroyImage(this->VKdevice->logicaldevice, offscreen.image, nullptr);
        irradianceCubeDescriptors->destroyDescriptor();
        vkDestroyPipeline(this->VKdevice->logicaldevice, pipeline, nullptr);
#else

#endif
    }
}