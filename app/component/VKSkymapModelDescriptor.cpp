#include "VKSkymapModelDescriptor.h"

namespace vkengine {
// 0번, 1번은 공통사항,  2번 이후 부터는 개별적
void VKSkyMapModelDescriptor::createDescriptorSetLayout(bool useTexture) {
    if (this->VKdescriptorSetLayout != VK_NULL_HANDLE) return;

    // Binding 0: Uniform buffer (Vertex shader)
    this->uboLayoutBinding[0].binding = 0;
    this->uboLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    this->uboLayoutBinding[0].descriptorCount = 1;
    this->uboLayoutBinding[0].stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    this->uboLayoutBinding[0].pImmutableSamplers = nullptr;

    // Binding 1: Texture sampler (cubemap Fragment shader)
    this->uboLayoutBinding[1].binding = 1;
    this->uboLayoutBinding[1].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    this->uboLayoutBinding[1].descriptorCount = 1;
    this->uboLayoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    this->uboLayoutBinding[1].pImmutableSamplers = nullptr;

    // Binding 2: Texture sampler (textureArray Fragment shader)
    if (useTexture) {
        this->uboLayoutBinding[2].binding = 2;
        this->uboLayoutBinding[2].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        this->uboLayoutBinding[2].descriptorCount = 1;
        this->uboLayoutBinding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        this->uboLayoutBinding[2].pImmutableSamplers = nullptr;
    }
    this->useTexture = useTexture;

    this->layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    this->layoutInfo.pBindings = &uboLayoutBinding[0];
    this->layoutInfo.bindingCount = useTexture ? 3 : 2; // 2개 바인딩이 필요
    this->layoutInfo.pNext = nullptr;

    this->buildDescriptorSetLayout();
  }

void VKSkyMapModelDescriptor::createDescriptorPool(bool useTexture) {

    if (this->VKdescriptorPool != VK_NULL_HANDLE || this->objects.empty())
        return;

    uint32_t objectCount = static_cast<uint32_t>(this->objects.size());

    //// 디스크립터 풀 크기를 설정합니다.
    this->poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    this->poolSizes[0].descriptorCount = objectCount * MAX_FRAMES_IN_FLIGHT;

    this->poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    this->poolSizes[1].descriptorCount = objectCount * MAX_FRAMES_IN_FLIGHT;

    if (useTexture)
    {
        this->poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        this->poolSizes[2].descriptorCount = objectCount * MAX_FRAMES_IN_FLIGHT;
    }

    // 디스크립터 풀 생성 정보 구조체를 초기화합니다.
    this->poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    //this->poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // 디스크립터 세트를 자유롭게 해제할 수 있도록 설정
    this->poolInfo.pNext = nullptr;
    this->poolInfo.poolSizeCount = useTexture ? 3 : 2; // 풀 사이즈 개수
    this->poolInfo.pPoolSizes = this->poolSizes;
    this->poolInfo.maxSets = objectCount * MAX_FRAMES_IN_FLIGHT; // 디스크립터 세트 개수

    this->buildDescriptorPool();
}

void VKSkyMapModelDescriptor::createDescriptorSets(bool useTexture) {

    if (!this->VKdescriptorSets.empty() || this->objects.empty())
        return;

    uint32_t objectCount = static_cast<uint32_t>(this->objects.size());

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * objectCount, this->VKdescriptorSetLayout);

    this->allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    this->allocInfo.descriptorPool = this->VKdescriptorPool;
    this->allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * objectCount);
    this->allocInfo.pSetLayouts = layouts.data();

    // 디스크립터 세트를 생성합니다.
    this->VKdescriptorSets.resize(MAX_FRAMES_IN_FLIGHT * objectCount);

    this->buildDescriptorSets();
}

void VKSkyMapModelDescriptor::updateDescriptorSets() {

    if (this->objects.empty())
        return;

    for (cUint16_t i = 0; i < this->objects.size(); i++)
    {
        // 디스크립터 버퍼 정보를 설정합니다.
        object::SkyBox* object = static_cast<object::SkyBox*>(this->objects[i]);

        VKcubeMap* cubeMap = reinterpret_cast<VKcubeMap*>(object->getTexture());

        cUint16_t offset = i * MAX_FRAMES_IN_FLIGHT; // 여러 오브젝트가 있을 경우 각 오브젝트의 오프셋을 계산

        for (cUint16_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            UniformBuffer* mvpBuffer = object->getModelViewProjUniformBuffer(j);

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = this->VKdescriptorSets[j + offset];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &mvpBuffer->descriptor;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = this->VKdescriptorSets[j + offset];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &cubeMap->imageInfo;

            if (this->useTexture)
            {
                /*descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[2].dstSet = this->VKdescriptorSets[i];
                descriptorWrites[2].dstBinding = 2;
                descriptorWrites[2].dstArrayElement = 0;
                descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[2].descriptorCount = 1;
                descriptorWrites[2].pImageInfo = &imageInfo2;*/
            }
            vkUpdateDescriptorSets(this->logicaldevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

}

void VKSkyMapModelDescriptor::BindDescriptorSets(
    VkCommandBuffer mainCommandBuffer, size_t currentFrame, uint16_t offset) {
    if (this->VKdescriptorSets.empty())
        return;

    vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKpipelineLayout, 0, 1, &this->VKdescriptorSets[currentFrame + offset * MAX_FRAMES_IN_FLIGHT], 0, nullptr);

}

void VKSkyMapModelDescriptor::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // 구조체 타입을 설정
    pipelineLayoutInfo.pNext = nullptr;                                       // 다음 구조체 포인터를 설정
    pipelineLayoutInfo.setLayoutCount = 1;                                    // 레이아웃 개수를 설정
    pipelineLayoutInfo.pSetLayouts = &this->VKdescriptorSetLayout;            // 레이아웃 포인터를 설정
    pipelineLayoutInfo.pushConstantRangeCount = 0;                            // 푸시 상수 범위 개수를 설정
    pipelineLayoutInfo.pPushConstantRanges = nullptr;                         // 푸시 상수 범위 포인터를 설정

    _VK_CHECK_RESULT_(vkCreatePipelineLayout(this->logicaldevice, &pipelineLayoutInfo, nullptr, &this->VKpipelineLayout));
}

}  // namespace vkengine