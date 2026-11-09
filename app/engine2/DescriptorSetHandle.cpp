#include "DescriptorSetHandle.h"

#include "helper.h"
#include "log.h"


namespace vkengine
{
    // ShaderResource 버전 create()는 VKResourceBinding의 정보를 ShaderResource의 updateBinding()과 updateWrite()로 대체하여 구현
    // 리소스를 세이더 binding 순서대로 받는다는 가정하에, 
    // 내부적으로 VKResourceBinding과 동일한 방식으로 layout binding 구성, descriptor set 할당, 업데이트 수행
    void DescriptorSetHandle::create(Context &context, const std::vector<std::reference_wrapper<ShaderResource>> &resourceBindings)
    {
        // 1. 각 리소스의 updateBinding()으로 layout bindings 구성
        //    binding 번호 = vector 내 순서 인덱스
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(resourceBindings.size());
        for (size_t i = 0; i < resourceBindings.size(); i++)
        {
            resourceBindings[i].get().updateBinding(layoutBindings[i]);
            layoutBindings[i].binding = uint32_t(i);
        }

        // 2. DescriptorManager2에서 매칭 layout 찾기 (SPIRV-Reflect stageFlags 포함)
        VkDescriptorSetLayout layout = context.getDescriptorManager()->getDescriptorSetLayout(layoutBindings);
        
        if (layout == VK_NULL_HANDLE)
        {
            Log::exitToLogger("Error: Failed to create descriptor set layout for material.");
        }
        
        // 완전한 layoutBindings (stageFlags 채워진 버전) 재취득
        layoutBindings = context.getDescriptorManager()->getBindingsForLayout(layout);

        // 3. descriptor set 할당
        descriptorSet = context.getDescriptorManager()->allocateDescriptorSet(layout);

        // 4. 각 리소스의 updateWrite()로 write 구성 후 일괄 업데이트
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(resourceBindings.size());

        for (size_t i = 0; i < resourceBindings.size(); i++)
        {
            VkWriteDescriptorSet write{};
            resourceBindings[i].get().updateWrite(write);

            // descriptorCount 가 0 인 write 는 Vulkan 이 금지한다
            // (VUID-VkWriteDescriptorSet-descriptorCount-arraylength).
            // 비어 있는 배열 리소스가 들어오면 그 바인딩만 건너뛴다. 레이아웃에
            // PARTIALLY_BOUND 가 켜져 있으므로 한 칸도 안 채운 채로 두어도 된다.
            if (write.descriptorCount == 0)
            {
                continue;
            }

            write.dstSet = descriptorSet;                 // DescriptorSetHandle 가 덮어쓴다
            write.dstBinding = layoutBindings[i].binding; // 걸러내도 바인딩 번호는 원래 i 의 것
            writes.push_back(write);
        }

        if (!writes.empty())
        {
            vkUpdateDescriptorSets(context.getDevice()->logicaldevice, uint32_t(writes.size()), writes.data(), 0, nullptr);
        }
    }

    const VkDescriptorSet &DescriptorSetHandle::get() const
    {
        if (descriptorSet == VK_NULL_HANDLE)
        {
            Log::exitToLogger("Warning: Attempting to access an uninitialized descriptor set.");
        }

        return descriptorSet;
    }

}