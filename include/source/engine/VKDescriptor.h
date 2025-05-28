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

        virtual void createDescriptorSetLayout(bool useTexture) = 0; // ���̴��� ������ ��ġ�� ���ҽ��� ���� �� �ְ� ���ִ� �������̽� ����
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

        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE }; // ���������� ���̾ƿ� -> ���������� ���̾ƿ��� ����

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
