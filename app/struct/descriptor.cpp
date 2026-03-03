#include "descriptor.h"

namespace vkengine
{
    const Bindinginfo* ShaderResourceLayout::findBinding(cUint32_t set, cUint32_t binding) const
    {
        for (const auto &descriptorSet : this->sets)
        {
            if (descriptorSet.setIndex == set)
            {
                for (const auto &bindingInfo : descriptorSet.bindings)
                {
                    if (bindingInfo.binding == binding)
                    {
                        return &bindingInfo;
                    }
                }
            }
        }
        return nullptr; // 해당 set과 binding을 찾지 못한 경우
    }

    const Bindinginfo* ShaderResourceLayout::findBindingByName(const cString &name) const
    {
        for (const auto &descriptorSet : this->sets)
        {
            for (const auto &bindingInfo : descriptorSet.bindings)
            {
                if (bindingInfo.name == name)
                {
                    return &bindingInfo;
                }
            }
        }
        return nullptr; // 해당 이름을 가진 바인딩을 찾지 못한 경우
    }

}