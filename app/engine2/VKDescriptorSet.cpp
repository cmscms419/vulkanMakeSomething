#include "VKDescriptorSet.h"

#include "helper.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{
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
            EXIT_TO_LOGGER("Error: Failed to create descriptor set layout for material.");
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