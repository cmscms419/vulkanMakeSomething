#include "triangle.h"
#include "../source/engine/helper.h"
#include "../source/engine/Camera.h"
#include "../source/engine/Debug.h"
#include "../source/struct.h"

using namespace vkengine::helper;
using namespace vkengine::debug;

using vkengine::object::Camera;

namespace vkengine {
    triangle::triangle(std::string root_path) : VulkanEngine(root_path) {}
    triangle::~triangle() {}

    void triangle::init()
    {
        VulkanEngine::init();

        this->camera = std::make_shared<vkengine::object::Camera>();
        this->camera->setProjection(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        this->camera->setView(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    bool triangle::prepare()
    {
        VulkanEngine::prepare();
        this->init_sync_structures();

        this->createVertexbuffer();
        this->createIndexBuffer();
        this->createUniformBuffers();

        this->createDescriptorSetLayout();
        this->createDescriptorPool();
        this->createDescriptorSets();

        this->createGraphicsPipeline();

        return true;
    }

    void triangle::cleanup()
    {
        if (this->_isInitialized)
        {
            this->cleanupSwapcChain();
            
            vkDestroyPipeline(this->VKdevice->VKdevice, this->VKgraphicsPipeline, nullptr);
            vkDestroyPipelineLayout(this->VKdevice->VKdevice, this->VKpipelineLayout, nullptr);
            vkDestroyRenderPass(this->VKdevice->VKdevice, *this->VKrenderPass.get(), nullptr);

            // 추가적인 부분
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroyBuffer(this->VKdevice->VKdevice, this->VKuniformBuffer[i].buffer, nullptr);
                vkFreeMemory(this->VKdevice->VKdevice, this->VKuniformBuffer[i].memory, nullptr);
            }

            vkDestroyDescriptorPool(this->VKdevice->VKdevice, this->VKdescriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(this->VKdevice->VKdevice, this->VKdescriptorSetLayout, nullptr);

            this->VKvertexBuffer.cleanup(this->VKdevice->VKdevice);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroySemaphore(this->VKdevice->VKdevice, this->VKframeData[i].VkimageavailableSemaphore, nullptr);
                vkDestroySemaphore(this->VKdevice->VKdevice, this->VKframeData[i].VkrenderFinishedSemaphore, nullptr);
                vkDestroyFence(this->VKdevice->VKdevice, this->VKframeData[i].VkinFlightFences, nullptr);
            }

            vkDestroyPipelineCache(this->VKdevice->VKdevice, this->VKpipelineCache, nullptr);

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

    void triangle::drawFrame()
    {
        // 렌더링을 시작하기 전에 프레임을 렌더링할 준비가 되었는지 확인합니다.
        VK_CHECK_RESULT(vkWaitForFences(this->VKdevice->VKdevice, 1, &this->getCurrnetFrameData().VkinFlightFences, VK_TRUE, UINT64_MAX));

        // 이미지를 가져오기 위해 스왑 체인에서 이미지 인덱스를 가져옵니다.
        // 주어진 스왑체인에서 다음 이미지를 획득하고, 
        // 선택적으로 세마포어와 펜스를 사용하여 동기화를 관리하는 Vulkan API의 함수입니다.
        uint32_t imageIndex = 0;
        VulkanEngine::prepareFame(&imageIndex);

        // uniform 버퍼를 업데이트합니다.
        this->updateUniformBuffer(static_cast<uint32_t>(this->currentFrame));
        
        // 플래그를 재설정합니다. -> 렌더링이 끝나면 플래그를 재설정합니다.
        vkResetFences(this->VKdevice->VKdevice, 1, &this->getCurrnetFrameData().VkinFlightFences); 

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
        vkResetFences(this->VKdevice->VKdevice, 1, &this->VKframeData[this->currentFrame].VkinFlightFences); 
        
        // 렌더링을 시작합니다.
        VK_CHECK_RESULT(vkQueueSubmit(this->VKdevice->graphicsVKQueue, 1, &VKsubmitInfo, this->VKframeData[this->currentFrame].VkinFlightFences));
        
        // 렌더링 종료 후, 프레젠트를 시작합니다.
        VulkanEngine::presentFrame(&imageIndex);

        this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    bool triangle::init_sync_structures()
    {
        VkSemaphoreCreateInfo semaphoreInfo = helper::semaphoreCreateInfo(0);
        VkFenceCreateInfo fenceInfo = helper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        for (auto& frameData : this->VKframeData)
        {
            VK_CHECK_RESULT(vkCreateSemaphore(this->VKdevice->VKdevice, &semaphoreInfo, nullptr, &frameData.VkimageavailableSemaphore));
            VK_CHECK_RESULT(vkCreateSemaphore(this->VKdevice->VKdevice, &semaphoreInfo, nullptr, &frameData.VkrenderFinishedSemaphore));
        }

        return true;
    }

    void triangle::recordCommandBuffer(FrameData* framedata, uint32_t imageIndex)
    {
        // 커맨드 버퍼 기록을 시작합니다.
        VkCommandBufferBeginInfo beginInfo = framedata->commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VK_CHECK_RESULT(vkBeginCommandBuffer(framedata->mainCommandBuffer, &beginInfo));

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
            // 그래픽 파이프라인을 바인딩합니다.
            vkCmdBindPipeline(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKgraphicsPipeline);

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

            // 버텍스 버퍼를 바인딩합니다.
            VkBuffer vertexBuffers[] = { this->VKvertexBuffer.vertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(framedata->mainCommandBuffer, 0, 1, vertexBuffers, offsets);

            // 인덱스 버퍼를 바인딩합니다.
            vkCmdBindIndexBuffer(framedata->mainCommandBuffer, this->VKvertexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

            // 디스크립터 세트를 바인딩합니다.
            vkCmdBindDescriptorSets(framedata->mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKpipelineLayout, 0, 1, &this->VKdescriptorSets[this->currentFrame], 0, nullptr);
            // 렌더 패스를 종료합니다.
            vkCmdDrawIndexed(framedata->mainCommandBuffer, static_cast<uint32_t>(testindices_.size()), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(framedata->mainCommandBuffer);

        // 커맨드 버퍼 기록을 종료합니다.
        VK_CHECK_RESULT(vkEndCommandBuffer(framedata->mainCommandBuffer));
    }

    void triangle::createVertexbuffer()
    {
        VkDeviceSize buffersize = sizeof(testVectex_[0]) * testVectex_.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        helper::createBuffer(
            this->VKdevice->VKdevice,
            this->VKdevice->VKphysicalDevice,
            buffersize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void* data;
        vkMapMemory(this->VKdevice->VKdevice, stagingBufferMemory, 0, buffersize, 0, &data);
            memcpy(data, testVectex_.data(), (size_t)buffersize);
        vkUnmapMemory(this->VKdevice->VKdevice, stagingBufferMemory);

        helper::createBuffer(
            this->VKdevice->VKdevice,
            this->VKdevice->VKphysicalDevice,
            buffersize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->VKvertexBuffer.vertexBuffer,
            this->VKvertexBuffer.vertexMemory);

        helper::copyBuffer(
            this->VKdevice->VKdevice,
            this->VKdevice->VKcommandPool,
            this->VKdevice->graphicsVKQueue,
            stagingBuffer,
            this->VKvertexBuffer.vertexBuffer,
            buffersize);

        // 버퍼 생성이 끝나면 스테이징 버퍼를 제거합니다.
        vkDestroyBuffer(this->VKdevice->VKdevice, stagingBuffer, nullptr);
        vkFreeMemory(this->VKdevice->VKdevice, stagingBufferMemory, nullptr);
    }

    void triangle::createIndexBuffer()
    {
        VkDeviceSize buffersize = sizeof(testindices_[0]) * testindices_.size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        
        helper::createBuffer(
            this->VKdevice->VKdevice,
            this->VKdevice->VKphysicalDevice,
            buffersize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);
        
        void* data;
        vkMapMemory(this->VKdevice->VKdevice, stagingBufferMemory, 0, buffersize, 0, &data);
        memcpy(data, testindices_.data(), (size_t)buffersize);
        vkUnmapMemory(this->VKdevice->VKdevice, stagingBufferMemory);
        
        helper::createBuffer(
            this->VKdevice->VKdevice,
            this->VKdevice->VKphysicalDevice,
            buffersize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->VKvertexBuffer.indexBuffer,
            this->VKvertexBuffer .indexmemory);
        
        helper::copyBuffer(
            this->VKdevice->VKdevice,
            this->VKdevice->VKcommandPool,
            this->VKdevice->graphicsVKQueue,
            stagingBuffer,
            this->VKvertexBuffer.indexBuffer,
            buffersize);
        
        // 버퍼 생성이 끝나면 스테이징 버퍼를 제거합니다.
        vkDestroyBuffer(this->VKdevice->VKdevice, stagingBuffer, nullptr);
        vkFreeMemory(this->VKdevice->VKdevice, stagingBufferMemory, nullptr);
    }

    void triangle::createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        this->VKuniformBuffer.resize(MAX_FRAMES_IN_FLIGHT);
        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            helper::createBuffer(
                this->VKdevice->VKdevice,
                this->VKdevice->VKphysicalDevice,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                this->VKuniformBuffer[i].buffer,
                this->VKuniformBuffer[i].memory);

            vkMapMemory(this->VKdevice->VKdevice, this->VKuniformBuffer[i].memory, 0, bufferSize, 0, &this->VKuniformBuffer[i].Mapped);
        }
    }

    void triangle::createDescriptorSetLayout()
    {
        // Binding 0: Uniform buffer (Vertex shader)
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        // Create the descriptor set layout
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = nullptr;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(this->VKdevice->VKdevice, &layoutInfo, nullptr, &this->VKdescriptorSetLayout));
    }

    void triangle::createDescriptorPool()
    {
        //// 디스크립터 풀 크기를 설정합니다.
        std::array<VkDescriptorPoolSize, 1> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        //poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        //poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        // 디스크립터 풀 생성 정보 구조체를 초기화합니다.
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VK_CHECK_RESULT(vkCreateDescriptorPool(this->VKdevice->VKdevice, &poolInfo, nullptr, &this->VKdescriptorPool));
    }

    void triangle::createDescriptorSets()
    {
        // 디스크립터 세트 레이아웃을 설정합니다.
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, this->VKdescriptorSetLayout);

        // 디스크립터 세트 할당 정보 구조체를 초기화합니다.
        VkDescriptorSetAllocateInfo allocInfo{};

        // 디스크립터 세트 할당 정보 구조체에 디스크립터 풀과 디스크립터 세트 개수를 설정합니다.
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = this->VKdescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        // 디스크립터 세트를 생성합니다.
        this->VKdescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        // 디스크립터 세트를 할당합니다.
        VK_CHECK_RESULT(vkAllocateDescriptorSets(this->VKdevice->VKdevice, &allocInfo, this->VKdescriptorSets.data()));

        // 디스크립터 세트를 설정합니다.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

            // 디스크립터 버퍼 정보를 설정합니다.
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = this->VKuniformBuffer[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = this->VKdescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(this->VKdevice->VKdevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void triangle::createGraphicsPipeline()
    {
        VkShaderModule baseVertshaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/vertTrinagle00.spv");
        VkShaderModule baseFragShaderModule = this->VKdevice->createShaderModule(this->RootPath + "../../../../../../shader/fragTrinagle00.spv");

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
        auto bindingDescription = VertexPosColor::getBindingDescription();
        auto attributeDescriptions = VertexPosColor::getAttributeDescriptions();

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

        // 고정 기능 상태를 설정합니다.
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

        VK_CHECK_RESULT(vkCreatePipelineLayout(this->VKdevice->VKdevice, &pipelineLayoutInfo, nullptr, &this->VKpipelineLayout));

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

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(this->VKdevice->VKdevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->VKgraphicsPipeline));

        vkDestroyShaderModule(this->VKdevice->VKdevice, baseVertshaderModule, nullptr);
        vkDestroyShaderModule(this->VKdevice->VKdevice, baseFragShaderModule, nullptr);
    }

    void triangle::updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};

        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = this->camera->getViewMatrix();
        //ubo.proj = glm::perspective(glm::radians(45.0f), this->VKswapChainExtent.width / (float)this->VKswapChainExtent.height, 0.1f, 10.0f);
        ubo.proj = this->camera->getProjectionMatrix();

        memcpy(this->VKuniformBuffer[currentImage].Mapped, &ubo, sizeof(ubo));
    }

    void triangle::cleanupSwapcChain()
    {
        this->VKdepthStencill.cleanup(this->VKdevice->VKdevice);

        for (auto framebuffers : this->VKswapChainFramebuffers)
        {
            vkDestroyFramebuffer(this->VKdevice->VKdevice, framebuffers, nullptr);
        }

        this->VKswapChain->cleanupSwapChain();
    }

}