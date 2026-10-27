#ifndef INCLUDE_VK_SHADERMANAGER_H_
#define INCLUDE_VK_SHADERMANAGER_H_

#include "Context.h"
#include "Shader.h"
#include "descriptor.h"

#include <map>
#include <unordered_map>
#include <vector>

namespace vkengine {

    class ShaderManager
    {
    public:
        ShaderManager(Context& ctx, cString path, const std::initializer_list<std::pair<cString, std::vector<cString>>>& pipelineShaders);

        ShaderManager(const ShaderManager&) = delete;

        ShaderManager& operator=(const ShaderManager&) = delete;

        ShaderManager& operator=(ShaderManager&&) = delete;

        void cleanup();

        VkPushConstantRange getPushConstantRange(cString pipelineName);

        std::vector<VkPipelineShaderStageCreateInfo> makePipelineShaderStageCIs(cString pipelineName) const;

        std::vector<VkVertexInputAttributeDescription> makeVertexInputAttrDesc(cString pipelineName) const;

        const std::unordered_map<cString, std::vector<Shader>>& getPipelineShaders() const;

        const std::vector<LayoutInfo>& getLayoutInfos() const;

        const ShaderResourceLayout& getShaderResourceLayout(cString pipelineName) const;

    private:
        Context& ctx;
        std::unordered_map<cString, std::vector<Shader>> pipelineShaders;
        std::unordered_map<cString, ShaderResourceLayout> Resourcelayouts; // 파이프라인별 레이아웃 정보 저장용
        std::vector<LayoutInfo> layoutInfos;

        // 쉐이더 생성 및 파이프라인별 쉐이더 관리
        void createFromShaders(cString path,
            std::initializer_list<std::pair<cString, std::vector<cString>>> pipelineShaders);

        // 파이프라인별 레이아웃 정보 수집
        void collectLayoutInfos();

        // 리플렉션된 데이터를 활용하여 Vulkan 디스크립터 레이아웃 정보를 만드는 함수
        void collectPerPipelineBindings(
            const cString& pipelineName,
            std::unordered_map<cString, ShaderResourceLayout>& bindingCollector) const;

        VkDescriptorSetLayoutBinding makeLayoutBindingFromReflect(const SpvReflectDescriptorBinding* binding,
            VkShaderStageFlagBits shaderStage) const;
    };

}

#endif // !INCLUDE_VK_SHADERMANAGER_H_
