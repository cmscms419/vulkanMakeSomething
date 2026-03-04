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

    // VKShaderResource 버전 create()는 VKResourceBinding의 정보를 VKShaderResource의 updateBinding()과 updateWrite()로 대체하여 구현
    // 리소스를 세이더 binding 순서대로 받는다는 가정하에, 
    // 내부적으로 VKResourceBinding과 동일한 방식으로 layout binding 구성, descriptor set 할당, 업데이트 수행
    void DescriptorSetHander::create(VKcontext &context, const std::vector<std::reference_wrapper<VKShaderResource>> &resourceBindings)
    {
        // 1. 각 리소스의 updateBinding()으로 layout bindings 구성
        //    binding 번호 = vector 내 순서 인덱스
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(resourceBindings.size());
        for (size_t i = 0; i < resourceBindings.size(); i++)
        {
            resourceBindings[i].get().updateBinding(layoutBindings[i]);
            layoutBindings[i].binding = uint32_t(i);
        }

        // 2. VKDescriptorManager2에서 매칭 layout 찾기 (SPIRV-Reflect stageFlags 포함)
        VkDescriptorSetLayout layout = context.getDescriptorManager()->getDescriptorSetLayout(layoutBindings);
        
        if (layout == VK_NULL_HANDLE)
        {
            EXIT_TO_LOGGER("Error: Failed to create descriptor set layout for material.\n");
        }
        
        // 완전한 layoutBindings (stageFlags 채워진 버전) 재취득
        layoutBindings = context.getDescriptorManager()->layoutToBindings(layout);

        // 3. descriptor set 할당
        descriptorSet = context.getDescriptorManager()->allocateDescriptorSet(layout);

        // 4. 각 리소스의 updateWrite()로 write 구성 후 일괄 업데이트
        std::vector<VkWriteDescriptorSet> writes(resourceBindings.size());
        for (size_t i = 0; i < resourceBindings.size(); i++)
        {
            resourceBindings[i].get().updateWrite(writes[i]);
            writes[i].dstSet = descriptorSet; // ← DescriptorSetHander가 덮어씀
            writes[i].dstBinding = layoutBindings[i].binding;
        }
        
        vkUpdateDescriptorSets(context.getDevice()->logicaldevice, uint32_t(writes.size()), writes.data(), 0, nullptr);
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