#include "imgui_skymap.h"

using namespace vkengine::helper;
using namespace vkengine::debug;

using vkengine::object::Camera;

namespace vkengine {

    imguiEngine::imguiEngine(std::string root_path) : VulkanEngine(root_path) {}
    imguiEngine::~imguiEngine() {}
    void imguiEngine::init()
    {
        VulkanEngine::init();
        this->camera = std::make_shared<vkengine::object::Camera>();
    }
    bool imguiEngine::prepare()
    {
        VulkanEngine::prepare();
        this->init_sync_structures();

        std::vector<std::string> pathArray = {
           this->RootPath + RESOURSE_PATH + TEST_TEXTURE_PATH_ARRAY0,
           this->RootPath + RESOURSE_PATH + TEST_TEXTURE_PATH_ARRAY1,
           this->RootPath + RESOURSE_PATH + TEST_TEXTURE_PATH_ARRAY2,
           this->RootPath + RESOURSE_PATH + TEST_TEXTURE_PATH_ARRAY3,
           this->RootPath + RESOURSE_PATH + TEST_TEXTURE_PATH_ARRAY4,
           this->RootPath + RESOURSE_PATH + TEST_TEXTURE_PATH_ARRAY5
        };

        this->cubeObject.createTextureArray(pathArray);

        std::vector<std::string> pathCubeArray = {
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/right.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/left.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/top.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/bottom.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/front.png",
           this->RootPath + RESOURSE_PATH + CUBE_TEXTURE_PATH + "/back.png"
        };

        this->cubeSkybox.createCubeMap(pathCubeArray);
        this->vkGUI = new vkengine::vkGUI(this);

        this->createVertexbuffer();
        this->createIndexBuffer();
        this->createUniformBuffers();

        this->createDescriptorSetLayout();
        this->createDescriptorPool();
        this->createDescriptorSets();

        this->createGraphicsPipeline();
        this->createGraphicsPipeline2();
        
        this->initUI();

        return true;
    }

    void imguiEngine::cleanup()
    {
        if (this->_isInitialized)
        {
            this->vkGUI->cleanup();
            this->cleanupSwapcChain();

            vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKgraphicsPipeline, nullptr);
            vkDestroyPipeline(this->VKdevice->logicaldevice, this->VKCubeMapPipeline, nullptr);

            vkDestroyPipelineLayout(this->VKdevice->logicaldevice, this->VKpipelineLayout, nullptr);
            vkDestroyRenderPass(this->VKdevice->logicaldevice, *this->VKrenderPass.get(), nullptr);

            // 추가적인 부분
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroyBuffer(this->VKdevice->logicaldevice, this->VKuniformBuffer[i].buffer, nullptr);
                vkFreeMemory(this->VKdevice->logicaldevice, this->VKuniformBuffer[i].memory, nullptr);
            }

            vkDestroyDescriptorPool(this->VKdevice->logicaldevice, this->VKdescriptorPoolMouse, nullptr);
            vkDestroyDescriptorPool(this->VKdevice->logicaldevice, this->VKdescriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(this->VKdevice->logicaldevice, this->VKdescriptorSetLayout, nullptr);

            this->cubeSkybox.cleanup();
            this->cubeObject.cleanup();

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

    void imguiEngine::drawFrame()
    {
        // 렌더링을 시작하기 전에 프레임을 렌더링할 준비가 되었는지 확인합니다.
        _VK_CHECK_RESULT_(vkWaitForFences(this->VKdevice->logicaldevice, 1, &this->getCurrnetFrameData().VkinFlightFences, VK_TRUE, UINT64_MAX));

        // 이미지를 가져오기 위해 스왑 체인에서 이미지 인덱스를 가져옵니다.
        // 주어진 스왑체인에서 다음 이미지를 획득하고, 
        // 선택적으로 세마포어와 펜스를 사용하여 동기화를 관리하는 Vulkan API의 함수입니다.
        uint32_t imageIndex = 0;
        VulkanEngine::prepareFame(&imageIndex);

        // uniform 버퍼를 업데이트합니다.
        this->updateUniformBuffer(static_cast<uint32_t>(this->currentFrame));

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

        this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    bool imguiEngine::mainLoop()
    {
        static auto currentTime = std::chrono::high_resolution_clock::now();

        while (!glfwWindowShouldClose(this->VKwindow)) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            this->update(frameTime);
            drawFrame();

#ifdef DEBUG_
            //printf("update\n");
#endif // DEBUG_

        }

        vkDeviceWaitIdle(this->VKdevice->logicaldevice);
        state = false;

        return state;
    }

    void imguiEngine::update(float dt)
    {
        if (this->m_keyPressed[GLFW_KEY_W])
        {
            this->camera->MoveForward(dt);
        }

        if (this->m_keyPressed[GLFW_KEY_S])
        {
            this->camera->MoveForward(-dt);
        }

        if (this->m_keyPressed[GLFW_KEY_A])
        {
            this->camera->MoveRight(-dt);
        }

        if (this->m_keyPressed[GLFW_KEY_D])
        {
            this->camera->MoveRight(dt);
        }

        if (this->m_keyPressed[GLFW_KEY_Q])
        {
            this->camera->MoveUp(-dt);
        }
        if (this->m_keyPressed[GLFW_KEY_E])
        {
            this->camera->MoveUp(dt);
        }

        //this->camera->setViewDirection(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        this->camera->update();
        float aspectRatio = static_cast<float>(this->VKswapChain->getSwapChainExtent().width) / static_cast<float>(VKswapChain->getSwapChainExtent().height);
        this->camera->setPerspectiveProjection(45.0f, aspectRatio, 0.1f, 100.0f);
    }

    bool imguiEngine::init_sync_structures()
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

    void imguiEngine::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
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

            vkCmdBindDescriptorSets(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKpipelineLayout, 0, 1, &this->VKdescriptorSets[this->currentFrame], 0, nullptr);

            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKCubeMapPipeline);
            this->cubeSkybox.draw(framedata->mainCommandBuffer, this->currentFrame);

            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);
            this->cubeObject.draw(framedata->mainCommandBuffer, this->currentFrame);

            this->vkGUI->begin();
            this->vkGUI->update();
            this->vkGUI->end();
        }

        vkCmdEndRenderPass(framedata->mainCommandBuffer);

        // 커맨드 버퍼 기록을 종료합니다.
        _VK_CHECK_RESULT_(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void imguiEngine::createVertexbuffer()
    {
        this->cubeObject.createVertexBuffer(const_cast<std::vector<Vertex>&>(cube));
        this->cubeSkybox.createVertexBuffer(const_cast<std::vector<Vertex>&>(skyboxVertices));
    }

    void imguiEngine::createIndexBuffer()
    {
        this->cubeObject.createIndexBuffer(const_cast<std::vector<uint16_t>&>(cubeindices_));
        this->cubeSkybox.createIndexBuffer(const_cast<std::vector<uint16_t>&>(skyboxIndices));
    }

    void imguiEngine::createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        this->VKuniformBuffer.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            helper::createBuffer(
                this->VKdevice->logicaldevice,
                this->VKdevice->physicalDevice,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                this->VKuniformBuffer[i].buffer,
                this->VKuniformBuffer[i].memory);

            vkMapMemory(this->VKdevice->logicaldevice, this->VKuniformBuffer[i].memory, 0, bufferSize, 0, &this->VKuniformBuffer[i].Mapped);
        }
    }

    void imguiEngine::createDescriptorSetLayout()
    {
        // Binding 0: Uniform buffer (Vertex shader)
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        // Binding 1: Texture sampler (cube map Fragment shader)
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;

        // Binding 2: Texture sampler (textureArray Fragment shader)
        VkDescriptorSetLayoutBinding samplerLayoutBinding2{};
        samplerLayoutBinding2.binding = 2;
        samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding2.descriptorCount = 1;
        samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding2.pImmutableSamplers = nullptr;

        std::array< VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, samplerLayoutBinding, samplerLayoutBinding2 };

        // Create the descriptor set layout
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pBindings = bindings.data();
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pNext = nullptr;

        _VK_CHECK_RESULT_(vkCreateDescriptorSetLayout(this->VKdevice->logicaldevice, &layoutInfo, nullptr, &this->VKdescriptorSetLayout));
    }

    void imguiEngine::createDescriptorPool()
    {
        // 디스크립터 풀 크기를 설정합니다.
        std::array<VkDescriptorPoolSize, MAX_FRAMES_IN_FLIGHT_UI_VERSION> poolSizes{};

        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        // 디스크립터 풀 생성 정보 구조체를 초기화합니다.
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        _VK_CHECK_RESULT_(vkCreateDescriptorPool(this->VKdevice->logicaldevice, &poolInfo, nullptr, &this->VKdescriptorPoolMouse));
    }

    void imguiEngine::createDescriptorSets()
    {
        // 디스크립터 세트 레이아웃을 설정합니다.
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, this->VKdescriptorSetLayout);

        // 디스크립터 세트 할당 정보 구조체를 초기화합니다.
        VkDescriptorSetAllocateInfo allocInfo{};

        // 디스크립터 세트 할당 정보 구조체에 디스크립터 풀과 디스크립터 세트 개수를 설정합니다.
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = this->VKdescriptorPoolMouse;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        // 디스크립터 세트를 생성합니다.
        this->VKdescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        // 디스크립터 세트를 할당합니다.
        _VK_CHECK_RESULT_(vkAllocateDescriptorSets(this->VKdevice->logicaldevice, &allocInfo, this->VKdescriptorSets.data()));

        // 디스크립터 세트를 설정합니다.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

            // 디스크립터 버퍼 정보를 설정합니다.
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = this->VKuniformBuffer[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = this->cubeSkybox.getCubeMap()->imageView;
            imageInfo.sampler = this->cubeSkybox.getCubeMap()->sampler;

            VkDescriptorImageInfo imageInfo2{};
            imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo2.imageView = this->cubeObject.getCubeTexture().imageView;
            imageInfo2.sampler = this->cubeObject.getCubeTexture().sampler;

            std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = this->VKdescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = this->VKdescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = this->VKdescriptorSets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pImageInfo = &imageInfo2;


            vkUpdateDescriptorSets(this->VKdevice->logicaldevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void imguiEngine::createGraphicsPipeline()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertReflect.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragReflect.spv");

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

        // 그래픽 파이프라인 레이아웃을 생성합니다.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // 입력 데이터를 어떤 형태로 조립할 것인지 결정합니다.
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // 삼각형 리스트로 설정
        inputAssembly.primitiveRestartEnable = VK_FALSE; // 프리미티브 재시작 비활성화

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

        // 뷰포트 설정
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewpport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // 래스터화 설정 -> 레스터화는 그래픽 파이프라인의 일부로 정점을 픽셀로 변환하는 프로세스입니다.
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;                   // 깊이 클램핑 비활성화
        rasterizer.rasterizerDiscardEnable = VK_FALSE;            // 래스터화 버림 비활성화
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;            // 다각형 모드를 채우기로 설정
        rasterizer.lineWidth = 1.0f;                              // 라인 너비를 1.0f로 설정
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;              // 후면 면을 제거
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;   // 전면 면을 반시계 방향으로 설정
        rasterizer.depthBiasEnable = VK_FALSE;                    // 깊이 바이어스 비활성화
        rasterizer.depthBiasConstantFactor = 0.0f;              // 깊이 바이어스 상수 요소를 0.0f로 설정
        rasterizer.depthBiasClamp = 0.0f;                       // 깊이 바이어스 클램프를 0.0f로 설정
        rasterizer.depthBiasSlopeFactor = 0.0f;                 // 깊이 바이어스 슬로프 요소를 0.0f로 설정

        // 다중 샘플링 설정
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //
        multisampling.sampleShadingEnable = VK_FALSE;               // 샘플링 쉐이딩 비활성화
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 래스터화 샘플 수를 1비트로 설정
        multisampling.minSampleShading = 1.0f;                      // 최소 샘플링을 1.0f로 설정 -> option
        multisampling.pSampleMask = nullptr;                        // 샘플 마스크를 nullptr로 설정 -> option
        multisampling.alphaToCoverageEnable = VK_FALSE;             // 알파 커버리지 비활성화 -> option
        multisampling.alphaToOneEnable = VK_FALSE;                  // 알파 원 비활성화 -> option

        // 깊이 스텐실 테스트 설정
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

        // 컬러 블렌딩 설정
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;     // 컬러 쓰기 마스크를 설정
        colorBlendAttachment.blendEnable = VK_FALSE;                        // 블렌딩을 비활성화
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     // 소스 컬러 블렌딩 팩터를 설정
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;    // 대상 컬러 블렌딩 팩터를 설정
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;                // 컬러 블렌딩 연산을 설정
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;     // 소스 알파 블렌딩 팩터를 설정
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;    // 대상 알파 블렌딩 팩터를 설정
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;                // 알파 블렌딩 연산을 설정

        // 컬러 블렌딩 상태 생성 정보 구조체를 초기화합니다.
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; // 구조체 타입을 설정
        colorBlending.logicOpEnable = VK_FALSE;                                       // 논리 연산을 비활성화
        colorBlending.logicOp = VK_LOGIC_OP_COPY;                                     // 논리 연산을 설정
        colorBlending.attachmentCount = 1;                                            // 컬러 블렌딩 첨부 개수를 설정
        colorBlending.pAttachments = &colorBlendAttachment;                           // 컬러 블렌딩 첨부 포인터를 설정
        colorBlending.blendConstants[0] = 0.0f;                                       // 블렌딩 상수를 설정
        colorBlending.blendConstants[1] = 0.0f;                                       // 블렌딩 상수를 설정
        colorBlending.blendConstants[2] = 0.0f;                                       // 블렌딩 상수를 설정
        colorBlending.blendConstants[3] = 0.0f;                                       // 블렌딩 상수를 설정

        // 다이나믹 상태 설정 -> 레스터화 상태를 동적으로 변경할 수 있습니다.
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // 그래픽 파이프라인 레이아웃을 생성합니다.
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // 구조체 타입을 설정
        pipelineLayoutInfo.setLayoutCount = 1;                                    // 레이아웃 개수를 설정
        pipelineLayoutInfo.pSetLayouts = &this->VKdescriptorSetLayout;            // 레이아웃 포인터를 설정
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // 푸시 상수 범위 개수를 설정
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // 푸시 상수 범위 포인터를 설정

        _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->VKdevice->logicaldevice, &pipelineLayoutInfo, nullptr, &this->VKpipelineLayout));

        // 그래픽 파이프라인 생성 정보 구조체를 초기화합니다.
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
        pipelineInfo.layout = this->VKpipelineLayout;
        pipelineInfo.renderPass = *this->VKrenderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->VKgraphicsPipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);
    }

    void imguiEngine::createGraphicsPipeline2()
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

        // 그래픽 파이프라인 레이아웃을 생성합니다.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // 입력 데이터를 어떤 형태로 조립할 것인지 결정합니다.
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // 삼각형 리스트로 설정
        inputAssembly.primitiveRestartEnable = VK_FALSE; // 프리미티브 재시작 비활성화

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

        // 뷰포트 설정
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewpport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // 래스터화 설정 -> 레스터화는 그래픽 파이프라인의 일부로 정점을 픽셀로 변환하는 프로세스입니다.
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;                   // 깊이 클램핑 비활성화
        rasterizer.rasterizerDiscardEnable = VK_FALSE;            // 래스터화 버림 비활성화
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;            // 다각형 모드를 채우기로 설정
        rasterizer.lineWidth = 1.0f;                              // 라인 너비를 1.0f로 설정
        rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;             // skybox는 앞면을 제거
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;   // 전면 면을 반시계 방향으로 설정
        rasterizer.depthBiasEnable = VK_FALSE;                    // 깊이 바이어스 비활성화
        rasterizer.depthBiasConstantFactor = 0.0f;              // 깊이 바이어스 상수 요소를 0.0f로 설정
        rasterizer.depthBiasClamp = 0.0f;                       // 깊이 바이어스 클램프를 0.0f로 설정
        rasterizer.depthBiasSlopeFactor = 0.0f;                 // 깊이 바이어스 슬로프 요소를 0.0f로 설정

        // 다중 샘플링 설정
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //
        multisampling.sampleShadingEnable = VK_FALSE;               // 샘플링 쉐이딩 비활성화
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 래스터화 샘플 수를 1비트로 설정
        multisampling.minSampleShading = 1.0f;                      // 최소 샘플링을 1.0f로 설정 -> option
        multisampling.pSampleMask = nullptr;                        // 샘플 마스크를 nullptr로 설정 -> option
        multisampling.alphaToCoverageEnable = VK_FALSE;             // 알파 커버리지 비활성화 -> option
        multisampling.alphaToOneEnable = VK_FALSE;                  // 알파 원 비활성화 -> option

        // 깊이 스텐실 테스트 설정
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_FALSE; // 깊이 쓰기 비활성화
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        // 컬러 블렌딩 설정
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;     // 컬러 쓰기 마스크를 설정
        colorBlendAttachment.blendEnable = VK_FALSE;                        // 블렌딩을 비활성화
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     // 소스 컬러 블렌딩 팩터를 설정
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;    // 대상 컬러 블렌딩 팩터를 설정
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;                // 컬러 블렌딩 연산을 설정
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;     // 소스 알파 블렌딩 팩터를 설정
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;    // 대상 알파 블렌딩 팩터를 설정
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;                // 알파 블렌딩 연산을 설정

        // 컬러 블렌딩 상태 생성 정보 구조체를 초기화합니다.
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; // 구조체 타입을 설정
        colorBlending.logicOpEnable = VK_FALSE;                                       // 논리 연산을 비활성화
        colorBlending.logicOp = VK_LOGIC_OP_COPY;                                     // 논리 연산을 설정
        colorBlending.attachmentCount = 1;                                            // 컬러 블렌딩 첨부 개수를 설정
        colorBlending.pAttachments = &colorBlendAttachment;                           // 컬러 블렌딩 첨부 포인터를 설정
        colorBlending.blendConstants[0] = 0.0f;                                       // 블렌딩 상수를 설정
        colorBlending.blendConstants[1] = 0.0f;                                       // 블렌딩 상수를 설정
        colorBlending.blendConstants[2] = 0.0f;                                       // 블렌딩 상수를 설정
        colorBlending.blendConstants[3] = 0.0f;                                       // 블렌딩 상수를 설정

        // 다이나믹 상태 설정 -> 레스터화 상태를 동적으로 변경할 수 있습니다.
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // 그래픽 파이프라인 레이아웃을 생성합니다.
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // 구조체 타입을 설정
        pipelineLayoutInfo.setLayoutCount = 1;                                    // 레이아웃 개수를 설정
        pipelineLayoutInfo.pSetLayouts = &this->VKdescriptorSetLayout;            // 레이아웃 포인터를 설정
        pipelineLayoutInfo.pushConstantRangeCount = 0;                            // 푸시 상수 범위 개수를 설정
        pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // 푸시 상수 범위 포인터를 설정

        // 그래픽 파이프라인 생성 정보 구조체를 초기화합니다.
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
        pipelineInfo.layout = this->VKpipelineLayout;
        pipelineInfo.renderPass = *this->VKrenderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(this->VKdevice->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->VKCubeMapPipeline));

        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->logicaldevice, baseFragShaderModule, nullptr);


    }

    void imguiEngine::updateUniformBuffer(uint32_t currentImage)
    {
        /*static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();*/

        UniformBufferObject ubo{};

        //ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        ubo.model = glm::mat4(1.0f);
        ubo.view = this->camera->getViewMatrix();
        ubo.proj = this->camera->getProjectionMatrix();

        memcpy(this->VKuniformBuffer[currentImage].Mapped, &ubo, sizeof(ubo));
    }

    void imguiEngine::cleanupSwapcChain()
    {
        this->VKdepthStencill.cleanup(this->VKdevice->logicaldevice);

        for (auto framebuffers : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice->logicaldevice, framebuffers, nullptr);
        }

        this->VKswapChain->cleanupSwapChain();
    }
    void imguiEngine::initUI()
    {
        this->vkGUI->init(200, 200);
        this->vkGUI->initResources(this->getRenderPass(), this->getDevice()->graphicsVKQueue, this->getRootPath());
    }
}