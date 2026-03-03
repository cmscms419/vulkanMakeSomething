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
    struct Bindinginfo
    {
        cString name;              // SPIR-V 바인딩 이름 (e.g. "uScene")
        cUint32_t set;             // 디스크립터 세트 번호
        VkDescriptorSetLayoutBinding binding; // Vulkan의 바인딩 정보 구조체
        // UBO, SSBO, CombinedSampler, etc.
        // 배열 크기
        // 어느 셰이더 스테이지에서 사용하는지
    };

    struct PushConstantinfo
    {
        VkShaderStageFlags stageFlags;
        cUint32_t offset;
        cUint32_t size;
        cString name;
    };

    struct VertexInputinfo
    {
        cUint32_t location;
        VkFormat format;
        cUint32_t offset;  // vertex struct 내 바이트 오프셋
        cString name; // e.g. "inPosition"
    };

    struct DescriptorSetLayout
    {
        cUint32_t setIndex;
        std::vector<Bindinginfo> bindings; // binding 번호 순 정렬
    };

    struct ShaderResourceLayout
    {
        cString pipelineName;
        std::vector<DescriptorSetLayout> sets;        // setIndex 순 정렬
        std::optional<PushConstantinfo> pushConstant; // 없으면 nullopt
        std::vector<VertexInputinfo> vertexInputs;    // compute면 비어있음

        const Bindinginfo *findBinding(cUint32_t set, cUint32_t binding) const;
        const Bindinginfo *findBindingByName(const cString &name) const;
    };

} // namespace vkengine

#endif // INCLUDE_DESCRIPTOR_H_