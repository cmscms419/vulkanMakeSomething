#ifndef INCLUDE_VKDESCRIPTOR_H_
#define INCLUDE_VKDESCRIPTOR_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKengine.h"
#include "VKbuffer.h"
#include "VKtexture.h"

namespace vkengine {

    struct VKDescriptor
    {

    public:
        VKDescriptor(VkDevice device = VK_NULL_HANDLE) : logicaldevice(device) {};

        void destroyDescriptor();

        virtual void createDescriptorSetLayout(bool useTexture) = 0; // 세이더가 지정된 위치의 리소스를 읽을 수 있게 해주는 인터페이스 제공
        virtual void createDescriptorPool(bool useTexture) = 0;
        virtual void createDescriptorSets(bool useTexture) = 0;
        virtual void updateDescriptorSets() = 0;
        virtual void BindDescriptorSets(VkCommandBuffer mainCommandBuffer, size_t currentFrame, uint16_t offset) = 0;

        void buildDescriptorSetLayout();
        void buildDescriptorPool();
        void buildDescriptorSets();

    protected:

        void destroyDescriptorSetLayout();
        void destroyDescriptorSets();
        void destroyDescriptorPool();
        void destroyPipelineLayouts();

        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE }; // 파이프라인 레이아웃 -> 파이프라인 레이아웃을 생성

        VkDescriptorSetLayout VKdescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};

        VkDescriptorPool VKdescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorPoolCreateInfo poolInfo{};

        std::vector<VkDescriptorSet> VKdescriptorSets{};
        VkDescriptorSetAllocateInfo allocInfo{};

        VkDevice logicaldevice{ VK_NULL_HANDLE };
    };

}

#endif // !INCLUDE_VKDESCRIPTOR_H_
