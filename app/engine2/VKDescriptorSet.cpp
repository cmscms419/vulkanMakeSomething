#include "VKDescriptorSet.h"

#include "helper.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{
    void DescriptorSetHander::create(VKcontext &context, const std::vector<std::reference_wrapper<VKResourceBinding>> &resourceBindings)
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(resourceBindings.size());

        for (size_t i = 0; i < resourceBindings.size(); ++i)
        {
            const VKResourceBinding &binding = resourceBindings[i].get();

            layoutBindings[i].binding = static_cast<cUint32_t>(i);
            layoutBindings[i].descriptorType = binding.descriptorType;
            layoutBindings[i].descriptorCount = binding.descriptorCount; // Assuming one descriptor per binding
            // layoutBindings[i].stageFlags = binding.stageFlags;
            layoutBindings[i].stageFlags = 0;
            layoutBindings[i].pImmutableSamplers = nullptr; // Not using immutable samplers
        }

        // bindings를 기반으로 Descriptor Set Layout 생성
        VkDescriptorSetLayout layout = context.getDescriptorManager()->getDescriptorSetLayout(layoutBindings);

        if (layout == VK_NULL_HANDLE)
        {
            EXIT_TO_LOGGER("Error: Failed to create descriptor set layout for material.\n");
        }

        layoutBindings = context.getDescriptorManager()->layoutToBindings(layout);

        // resourceBindings의 정보를 기반으로 Descriptor Set 할당
        // 셰이더에서 결정
        for (size_t i = 0; i < resourceBindings.size(); i++)
        {
            resourceBindings[i].get().stageFlags = layoutBindings[i].stageFlags;
        }

        this->descriptorSet = context.getDescriptorManager()->allocateDescriptorSet(layout);

        // Descriptor Set 업데이트
        std::vector<VkWriteDescriptorSet> writeDescriptorSets(resourceBindings.size());

        for (size_t i = 0; i < resourceBindings.size(); i++)
        {
            const VKResourceBinding &bindingData = resourceBindings[i].get();
            VkWriteDescriptorSet &writeSet = writeDescriptorSets[i];

            writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeSet.pNext = nullptr;
            writeSet.dstSet = this->descriptorSet;
            writeSet.dstBinding = layoutBindings[i].binding;
            writeSet.dstArrayElement = 0;
            writeSet.descriptorType = layoutBindings[i].descriptorType;
            writeSet.descriptorCount = layoutBindings[i].descriptorCount;
            writeSet.pBufferInfo = bindingData.buffer ? &bindingData.bufferInfo : nullptr;
            writeSet.pImageInfo = bindingData.image ? &bindingData.imageInfo : nullptr;
            writeSet.pTexelBufferView = nullptr;
        }

        if (!writeDescriptorSets.empty())
        {
            vkUpdateDescriptorSets(context.getDevice()->logicaldevice, static_cast<cUint32_t>(writeDescriptorSets.size()),
                                   writeDescriptorSets.data(), 0, nullptr);
        }
    }

    const VkDescriptorSet &DescriptorSetHander::get() const
    {
        if (descriptorSet == VK_NULL_HANDLE)
        {
            EXIT_TO_LOGGER("Warning: Attempting to access an uninitialized descriptor set.");
        }

        return descriptorSet;
    }

}