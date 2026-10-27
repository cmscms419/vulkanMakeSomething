#ifndef INCLUDE_DESCRIPTOR_H_
#define INCLUDE_DESCRIPTOR_H_

#include "common.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <optional>

struct LayoutInfo
{
    std::vector<VkDescriptorSetLayoutBinding> bindings{};
    std::vector<std::tuple<cString, cUint32_t>> pipelineNamesAndSetNumbers;
};

namespace vkengine
{
    struct BindingBase
    {
        cString name; // SPIR-V 바인딩 이름 (e.g. "uScene")
    };

    struct BindingInfo : public BindingBase
    {
        VkDescriptorSetLayoutBinding binding; // Vulkan의 바인딩 정보 구조체
    };

    struct PushConstantInfo : public BindingBase
    {
        VkPushConstantRange range; // Vulkan의 푸시 상수 범위 구조체
    };

    struct VertexInputinfo : public BindingBase
    {
        VkVertexInputAttributeDescription attributeDescription; // Vulkan의 정점 입력 속성 설명 구조체
    };

    struct DescriptorSetLayout
    {
        std::unordered_map<cUint32_t, BindingInfo> bindings; // key : binding index, value: 해당 바인딩의 정보
    };

    struct ShaderResourceLayout
    {
        cString pipelineName;
        std::unordered_map<cUint32_t, DescriptorSetLayout> sets; // key: set index, value: 해당 set의 바인딩 정보
        std::optional<PushConstantInfo> pushConstant; // 없으면 nullopt
        std::vector<VertexInputinfo> vertexInputs;    // compute면 비어있음

        const BindingInfo *findBinding(cUint32_t set, cUint32_t binding) const;
        const BindingInfo *findBindingByName(const cString &name) const;
    };

    struct BindingBaseHash
    {
        size_t operator()(const std::vector<BindingBase> &bindings) const;
    };

    struct BindingBaseEqual
    {
        bool operator()(const std::vector<BindingBase> &lhs, const std::vector<BindingBase> &rhs) const;

    };
    
} // namespace vkengine

#endif // INCLUDE_DESCRIPTOR_H_
