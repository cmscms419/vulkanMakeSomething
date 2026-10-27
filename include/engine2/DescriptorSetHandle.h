#ifndef INCLUDE_VK_DESCRIPTOR_SET_H_
#define INCLUDE_VK_DESCRIPTOR_SET_H_

#include "common.h"

#include "Context.h"
#include "ShaderResource.h"

#include <optional>

namespace vkengine {
    class DescriptorSetHandle {

    public:
        // 리소스 해제에 대한 책임이 없기 때문에 Context를 멤버로 갖고 있을 필요가 없음
        void create(Context& context, const std::vector<std::reference_wrapper< ShaderResource>>& resourceBindings);
        const VkDescriptorSet& get() const;

    private:
        VkDescriptorSet descriptorSet;
    };
}

#endif // !INCLUDE_VK_DESCRIPTOR_SET_H_
