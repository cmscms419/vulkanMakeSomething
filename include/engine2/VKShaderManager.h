#ifndef INCLUDE_VK_SHADERMANAGER_H_
#define INCLUDE_VK_SHADERMANAGER_H_

#include "VKcontext.h"
#include "VKShader.h"

#include <map>
#include <unordered_map>
#include <vector>

namespace vkengine {

    class VKShaderManager
    {
    public:
        VKShaderManager(VKcontext& ctx, cString path, const std::initializer_list<std::pair<cString, std::vector<cString>>>& pipelineShaders);
        
        VKShaderManager(const VKShaderManager&) = delete;

        VKShaderManager& operator=(const VKShaderManager&) = delete;

        VKShaderManager& operator=(VKShaderManager&&) = delete;

        void cleanup();

        VkPushConstantRange pushConstantsRange(cString pipelineName);

        std::vector<VkPipelineShaderStageCreateInfo> createPipelineShaderStageCIs(cString pipelineName) const;

        std::vector<VkVertexInputAttributeDescription> createVertexInputAttrDesc(cString pipelineName) const;

        /*std::vector<VkDescriptorSetLayoutBinding> collectPerPipelineBindings() const;*/

        const std::unordered_map<cString, std::vector<VKshader>>& getPipelineShaders() const;

        const std::vector<LayoutInfo>& getLayoutInfos() const;

    private:
        VKcontext& ctx;
        std::unordered_map<cString, std::vector<VKshader>> pipelineShaders;
        std::vector<LayoutInfo> layoutInfos;

        // 쉐이더 생성 및 파이프라인별 쉐이더 관리
        void createFromShaders(cString path,
            std::initializer_list<std::pair<cString, std::vector<cString>>> pipelineShaders);

        // 파이프라인별 레이아웃 정보 수집
        void collectLayoutInfos();

        // 리플렉션된 데이터를 활용하여 Vulkan 디스크립터 레이아웃 정보를 만드는 함수
        void collectPerPipelineBindings(
            const cString& pipelineName,
            std::map<cUint32_t, std::map<cUint32_t, VkDescriptorSetLayoutBinding>>& bindingCollector) const;

        VkDescriptorSetLayoutBinding createLayoutBindingFromReflect(const SpvReflectDescriptorBinding* binding,
            VkShaderStageFlagBits shaderStage) const;
    };

}


#endif // !INCLUDE_VK_SHADERMANAGER_H_
