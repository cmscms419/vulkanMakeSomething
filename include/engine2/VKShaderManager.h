#ifndef INCLUDE_VK_SHADERMANAGER_H_
#define INCLUDE_VK_SHADERMANAGER_H_

#include "VKcontext.h"
#include "VKShader.h"

namespace vkengine {

    class VKShaderManager
    {
    public:
        VKShaderManager(VKcontext& ctx, cString path,
            const std::initializer_list<std::pair<cString, std::vector<cString>>>& pipelineShaders);
        VKShaderManager(const VKShaderManager&) = delete;

        VKShaderManager& operator=(const VKShaderManager&) = delete;

        VKShaderManager& operator=(VKShaderManager&&) = delete;

        void cleanup();

        VkPushConstantRange pushConstantsRange(cString pipelineName)
        {
            const std::vector<VKshader>& shaders = this->pipelineShaders.at(pipelineName);

            // Search through all shaders in the pipeline for push constants
            for (const auto& shader : shaders) {
                const auto& reflectModule = shader.reflectModule;

                // Check if this shader has push constants
                if (reflectModule.push_constant_block_count > 0) {
                    const SpvReflectBlockVariable* pushBlock = &reflectModule.push_constant_blocks[0];

                    VkPushConstantRange pushConstantRange{};
                    pushConstantRange.stageFlags = static_cast<VkShaderStageFlags>(shader.stage);
                    pushConstantRange.offset = 0;
                    pushConstantRange.size = pushBlock->size;

                    // Accumulate stage flags from other shaders that also use push constants
                    for (const auto& otherShader : shaders) {
                        if (otherShader.reflectModule.push_constant_block_count > 0) {
                            pushConstantRange.stageFlags |=
                                static_cast<VkShaderStageFlags>(otherShader.stage);
                        }
                    }

                    return pushConstantRange;
                }
            }
            
            // Return empty range if no push constants found
            VkPushConstantRange emptyRange{};
            emptyRange.stageFlags = 0;
            emptyRange.offset = 0;
            emptyRange.size = 0;
            return emptyRange;
        }

        std::vector<VkPipelineShaderStageCreateInfo> createPipelineShaderStageCIs(cString pipelineName) const;

        std::vector<VkVertexInputAttributeDescription> createVertexInputAttrDesc(cString pipelineName) const;

        /*std::vector<VkDescriptorSetLayoutBinding> collectPerPipelineBindings() const;*/

        const std::unordered_map<cString, std::vector<VKshader>>& getPipelineShaders() const
        {
            return pipelineShaders;
        }

        const std::vector<LayoutInfo>& getLayoutInfos() const
        {
            return layoutInfos;
        }

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
