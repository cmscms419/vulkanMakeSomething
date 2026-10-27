#include "descriptor.h"

namespace vkengine
{
    const BindingInfo* ShaderResourceLayout::findBinding(cUint32_t set, cUint32_t binding) const
    {
        if (this->sets.find(set) != this->sets.end())
        {
            const auto& descriptorSet = this->sets.at(set);
            if (descriptorSet.bindings.find(binding) != descriptorSet.bindings.end())
            {
                return &descriptorSet.bindings.at(binding);
            }
        }

        return nullptr; // 해당 set과 binding을 찾지 못한 경우
    }

    const BindingInfo* ShaderResourceLayout::findBindingByName(const cString &name) const
    {
        for (const auto &descriptorSet : this->sets)
        {
            for (const auto &bindingInfo : descriptorSet.second.bindings) // descriptorSet.second는 DescriptorSetLayout 객체
            {
                if (bindingInfo.second.name == name)
                {
                    return &bindingInfo.second; // bindingInfo.second는 BindingInfo 객체
                }
            }
        }
        return nullptr; // 해당 이름을 가진 바인딩을 찾지 못한 경우
    }

    size_t BindingBaseHash::operator()(const std::vector<BindingBase> &bindings) const
    {
        size_t hash = 0;

        for (const auto &binding : bindings)
        {
            hash ^= std::hash<cString>{}(binding.name) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }

        return hash;
    }

    bool BindingBaseEqual::operator()(const std::vector<BindingBase> &lhs, const std::vector<BindingBase> &rhs) const
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (size_t i = 0; i < lhs.size(); ++i)
        {
            if (lhs[i].name != rhs[i].name)
            {
                return false;
            }
        }

        return true;
    }

}