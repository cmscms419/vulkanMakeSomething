 #include "VKDescriptor.h"

#include "VKengine.h"

namespace vkengine {

    void VKDescriptor::destroyDescriptor()
    {
        VKDescriptor::destroyDescriptorSetLayout();
        VKDescriptor::destroyDescriptorPool();
        VKDescriptor::destroyPipelineLayouts();
    }

    void VKDescriptor::buildDescriptorSetLayout()
    {
        if (this->logicaldevice == VK_NULL_HANDLE)
            return;

        _VK_CHECK_RESULT_(vkCreateDescriptorSetLayout(this->logicaldevice, &layoutInfo, nullptr, &this->VKdescriptorSetLayout));

    }

    void VKDescriptor::buildDescriptorPool()
    {
        if (this->logicaldevice == VK_NULL_HANDLE)
            return;

        _VK_CHECK_RESULT_(vkCreateDescriptorPool(this->logicaldevice, &poolInfo, nullptr, &this->VKdescriptorPool));
    }

    void VKDescriptor::buildDescriptorSets()
    {
        if (this->logicaldevice == VK_NULL_HANDLE)
            return;

        _VK_CHECK_RESULT_(vkAllocateDescriptorSets(this->logicaldevice, &allocInfo, this->VKdescriptorSets.data()));
    }

    void VKDescriptor::destroyDescriptorSetLayout()
    {
        if (this->VKdescriptorSetLayout == VK_NULL_HANDLE)
            return;

        vkDestroyDescriptorSetLayout(this->logicaldevice, this->VKdescriptorSetLayout, nullptr);
        this->VKdescriptorSetLayout = VK_NULL_HANDLE;
    }

    void VKDescriptor::destroyDescriptorPool()
    {
        if (this->VKdescriptorPool == VK_NULL_HANDLE)
            return;

        vkDestroyDescriptorPool(this->logicaldevice, this->VKdescriptorPool, nullptr);
        this->VKdescriptorPool = VK_NULL_HANDLE;
    }

    void VKDescriptor::destroyDescriptorSets()
    {
        if (this->VKdescriptorSets.size() > 0)
        {
            for (auto& descriptorSet : this->VKdescriptorSets)
            {
                vkFreeDescriptorSets(this->logicaldevice, this->VKdescriptorPool, 1, &descriptorSet);
            }
            this->VKdescriptorSets.clear();
        }
    }

    void VKDescriptor::destroyPipelineLayouts()
    {
        if (this->VKpipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(this->logicaldevice, this->VKpipelineLayout, nullptr);
            this->VKpipelineLayout = VK_NULL_HANDLE;
        }
    }

    void VKDescriptor2::destroyDescriptorSetLayout()
    {
        if (this->logicaldevice == VK_NULL_HANDLE)
            return;

        vkDestroyDescriptorSetLayout(this->logicaldevice, this->VKdescriptorSetLayout, nullptr);
    }

    void VKDescriptor2::destroyDescriptorSets()
    {
        if (this->logicaldevice == VK_NULL_HANDLE)
            return;

        _VK_CHECK_RESULT_(vkAllocateDescriptorSets(this->logicaldevice, &allocInfo, this->VKdescriptorSets.data()));
    }

    void VKDescriptor2::destroyDescriptorPool()
    {
        if (this->logicaldevice == VK_NULL_HANDLE)
            return;

        vkDestroyDescriptorPool(this->logicaldevice, this->VKdescriptorPool, nullptr);
    }

    void vkengine::VKDescriptor2::destroyPipelineLayouts()
    {
        if (this->VKpipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(this->logicaldevice, this->VKpipelineLayout, nullptr);
            this->VKpipelineLayout = VK_NULL_HANDLE;
        }
    }

    void VKDescriptor2::destroyDescriptor()
    {
        this->destroyDescriptorSetLayout();
        this->destroyDescriptorPool();
        this->destroyPipelineLayouts();
    }

    void VKDescriptor2::BindDescriptorSets(
        VkCommandBuffer mainCommandBuffer, size_t currentFrame, uint16_t offset)
    {
        if (this->VKdescriptorSets.empty())
            return;

        vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->VKpipelineLayout, 0,
            1, &this->VKdescriptorSets[currentFrame + offset * this->frames], 0, nullptr);
    }

}