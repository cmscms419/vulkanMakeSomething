#include "VKnormalRander.h"

namespace vkengine
{
    namespace helper
    {
        void normalRander::cleanup()
        {
            if (this->normalRanderObject)
            {
                this->normalRanderObject->cleanup();
            }
            for (int i = 0; i < 2; i++)
            {
                this->normalRanderUniformBuffer[i].cleanup();
            }
            this->cleanupNormalRanderPipeline();
        }

        void normalRander::createNormalObject(vkengine::object::Object3d* object)
        {
            std::vector<Vertex> normalVertices;
            std::vector<uint16_t> normalIndices;
            std::vector<Vertex>* vertices = object->getVertices();

            for (size_t i = 0; i < vertices->size(); i++)
            {
                Vertex vertex = (*vertices)[i];

                vertex.texCoord.x = 0.0f; // 시작
                normalVertices.push_back(vertex);

                vertex.texCoord.x = 1.0f; // 끝
                normalVertices.push_back(vertex);

                normalIndices.push_back(uint16_t(2 * i));
                normalIndices.push_back(uint16_t(2 * i + 1));
            }

            this->normalRanderObject = new vkengine::object::ModelObject(engine->getDevice());
            this->normalRanderObject->setName("Normal Rander Object3d");
            this->normalRanderObject->setVertices(normalVertices);
            this->normalRanderObject->setIndices(normalIndices);
            this->normalRanderObject->setMatrix(object->getMatrix());
            
            this->normalRanderObject->createVertexBuffer();
            this->normalRanderObject->createIndexBuffer();
            this->normalRanderObject->createModelViewProjBuffers();
            this->normalRanderObject->getTexture()->createDescriptorImageInfo();
            this->normalRanderObject->getModelViewProjUniformBuffer(0)->createDescriptorBufferInfo();
            this->normalRanderObject->getModelViewProjUniformBuffer(1)->createDescriptorBufferInfo();

        }

        void normalRander::createNormalRanderPipeline()
        {

            normalRanderDescriptor = new VKDescriptor2(engine->getDevice()->logicaldevice, 2);
            cUint16_t imageCount = 2;

            // pool create
            std::vector<VkDescriptorPoolSize> PoolSizes = {
                helper::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, imageCount) // UBO
            };
            this->normalRanderDescriptor->poolInfo = helper::descriptorPoolCreateInfo(PoolSizes, imageCount);
            this->normalRanderDescriptor->createDescriptorPool();

            // layout create
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                helper::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0), // 카메라
            };
            this->normalRanderDescriptor->layoutInfo = helper::descriptorSetLayoutCreateInfo(setLayoutBindings);
            this->normalRanderDescriptor->createDescriptorSetLayout();

            // descriptor set create
            std::vector<VkDescriptorSetLayout> layouts(
                this->normalRanderDescriptor->frames,
                this->normalRanderDescriptor->VKdescriptorSetLayout);
            this->normalRanderDescriptor->allocInfo = helper::descriptorSetAllocateInfo(
                this->normalRanderDescriptor->VKdescriptorPool, 
                *layouts.data(),
                imageCount
                );

            this->normalRanderDescriptor->VKdescriptorSets.resize(imageCount);
            this->normalRanderDescriptor->createAllocateDescriptorSets();

            std::vector<VkWriteDescriptorSet> writeDescriptorSets01 = {
                helper::writeDescriptorSet(
                    this->normalRanderDescriptor->VKdescriptorSets[0],
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    0,
                    &this->normalRanderObject->getModelViewProjUniformBuffer(0)->descriptor)
            };

            std::vector<VkWriteDescriptorSet> writeDescriptorSets02 = {
                helper::writeDescriptorSet(
                    this->normalRanderDescriptor->VKdescriptorSets[1],
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    0,
                    &this->normalRanderObject->getModelViewProjUniformBuffer(1)->descriptor)
            };

            vkUpdateDescriptorSets(
                engine->getDevice()->logicaldevice,
                static_cast<uint32_t>(writeDescriptorSets01.size()),
                writeDescriptorSets01.data(),
                0, nullptr);

            vkUpdateDescriptorSets(
                engine->getDevice()->logicaldevice,
                static_cast<uint32_t>(writeDescriptorSets02.size()),
                writeDescriptorSets02.data(),
                0, nullptr);

            // Normal Rander 파이프라인 생성 로직 구현
            VkShaderModule vertShader = engine->getDevice()->createShaderModule(
                engine->getRootPath() + "../../../../../../shader/vertNormalShader.spv");
            VkShaderModule fragShader = engine->getDevice()->createShaderModule(
                engine->getRootPath() + "../../../../../../shader/fragNormalShader.spv");

            VkPipelineShaderStageCreateInfo shaderStages[2] = {
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShader, "main"),
            helper::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader, "main")
            };

            // 2. Vertex input, binding, attribute 등 기존 모델과 동일하게 설정
            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();

            VkPipelineVertexInputStateCreateInfo vertexInputInfo = helper::pipelineVertexInputStateCreateInfo(
                bindingDescription, *attributeDescriptions.data(), static_cast<uint32_t>(attributeDescriptions.size()), 1);
            VkPipelineInputAssemblyStateCreateInfo inputAssembly = helper::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 0, VK_FALSE);

            VkViewport viewpport{};
            viewpport.x = 0.0f;
            viewpport.y = 0.0f;
            viewpport.width = (float)engine->getSwapChain()->getSwapChainExtent().width;
            viewpport.height = (float)engine->getSwapChain()->getSwapChainExtent().height;
            viewpport.minDepth = 0.0f;
            viewpport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = engine->getSwapChain()->getSwapChainExtent();

            // 4. 파이프라인 레이아웃 생성
            VkPipelineViewportStateCreateInfo viewportState = helper::pipelineViewportStateCreateInfo(viewpport, scissor, 1, 1);
            VkPipelineRasterizationStateCreateInfo rasterizer = helper::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            VkPipelineMultisampleStateCreateInfo multisampling = helper::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
            VkPipelineDepthStencilStateCreateInfo depthStencil = helper::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
            VkPipelineColorBlendAttachmentState colorBlendAttachment = helper::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
            VkPipelineColorBlendStateCreateInfo colorBlending = helper::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
            VkPipelineDynamicStateCreateInfo dynamicState = helper::pipelineDynamicStateCreateInfo(dynamicStates);

            std::vector<VkPushConstantRange> pushConstantRanges = {
                helper::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(float), 0), // 푸시 상수 범위 설정
            };

            VkPipelineLayoutCreateInfo pipelineLayoutInfo = helper::pipelineLayoutCreateInfo(
                &this->normalRanderDescriptor->VKdescriptorSetLayout, 1);

            pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
            pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

            _VK_CHECK_RESULT_(
                vkCreatePipelineLayout(
                    engine->getDevice()->logicaldevice, 
                    &pipelineLayoutInfo, 
                    nullptr, &this->normalRanderDescriptor->VKpipelineLayout));

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
            pipelineInfo.layout = this->normalRanderDescriptor->VKpipelineLayout;
            pipelineInfo.renderPass = engine->getRenderPass(); // 렌더 패스 설정
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
            pipelineInfo.basePipelineIndex = -1; // Optional

            _VK_CHECK_RESULT_(vkCreateGraphicsPipelines(engine->getDevice()->logicaldevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->normalRanderPipeline));

            // 셰이더 모듈 정리
            vkDestroyShaderModule(engine->getDevice()->logicaldevice, vertShader, nullptr);
            vkDestroyShaderModule(engine->getDevice()->logicaldevice, fragShader, nullptr);

        }

        void normalRander::recordNormalRanderCommandBuffer(FrameData* frameData, uint32_t imageIndex)
        {
            VkDeviceSize offsets[] = { 0 };
            // Normal Rander 커맨드 버퍼 레코드 로직 구현

            this->normalRanderDescriptor->BindDescriptorSets(frameData->mainCommandBuffer,this->engine->getCurrentFrame(),0);
            vkCmdBindPipeline(frameData->mainCommandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,this->normalRanderPipeline);
            vkCmdPushConstants(frameData->mainCommandBuffer, this->normalRanderDescriptor->VKpipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &this->normalRanderObjectScale);
            vkCmdBindVertexBuffers(frameData->mainCommandBuffer, 0, 1, &this->normalRanderObject->getVertexBuffer()->buffer, offsets);
            vkCmdBindIndexBuffer(frameData->mainCommandBuffer, this->normalRanderObject->getIndexBuffer()->buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(frameData->mainCommandBuffer, static_cast<uint32_t>(this->normalRanderObject->getIndices()->size()), 1, 0, 0, 0);

        }

        void normalRander::cleanupNormalRanderPipeline()
        {
            // Normal Rander 파이프라인 정리 로직 구현
            if (this->normalRanderPipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(this->engine->getDevice()->logicaldevice, this->normalRanderPipeline, nullptr);
                this->normalRanderPipeline = VK_NULL_HANDLE;
            }

            if (this->normalRanderDescriptor)
            {
                this->normalRanderDescriptor->destroyDescriptor();
                delete this->normalRanderDescriptor;
                this->normalRanderDescriptor = nullptr;
            }
        }

        void normalRander::updateCameraUniformBuffer(uint32_t currentImage, cMat4 world, vkengine::object::Camera* camera)
        {

#if 1
            UniformBufferObject ubo{};

            ubo.model = world * this->normalRanderObject->getMatrix();
            ubo.inverseTranspose = glm::transpose(glm::inverse(this->normalRanderObject->getMatrix())); // 역행렬의 전치 행렬
            ubo.view = camera->getViewMatrix();
            ubo.proj = camera->getProjectionMatrix();

            memcpy(this->normalRanderObject->getModelViewProjUniformBuffer(currentImage)->mapped, &ubo, sizeof(ubo));
#else
            this->normalRanderObject->updateUniformBuffer(currentImage, world, camera);
#endif
        }
    }
}