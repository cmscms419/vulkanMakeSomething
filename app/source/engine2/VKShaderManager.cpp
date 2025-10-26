#include "VKShaderManager.h"
using namespace vkengine::Log;

namespace vkengine {
    VKShaderManager::VKShaderManager(VkContext& ctx, cString shaderPathPrefix, const std::initializer_list<std::pair<cString, std::vector<cString>>>& pipelineShaders)
        : ctx(ctx)
    {
        // shahder 정보를 읽어서 저장
        createFromShaders(shaderPathPrefix, pipelineShaders);

        collectLayoutInfos();
    }

    void VKShaderManager::createFromShaders(cString path, std::initializer_list<std::pair<cString, std::vector<cString>>> pipelineShaders)
    {
        for (const auto& [pipelineName, shaderFiles] : pipelineShaders) {
            std::vector<VKshader>& shaders = this->pipelineShaders[pipelineName];
            shaders.reserve(shaderFiles.size());

            for (cString shaderFile : shaderFiles)
            {
                shaderFile = path + shaderFile;

                if (shaderFile.substr(shaderFile.length() - 4) != ".spv") {
                    shaderFile += ".spv";
                }

                shaders.emplace_back(VKshader(this->ctx, shaderFile));

            }
        }
    }
    void VKShaderManager::collectLayoutInfos()
    {
        // 모든 파이프라인에 대해 레이아웃 정보를 수집하기 위한 컨테이너
        // Key: 정규화된 바인딩 벡터 (stageFlags=0), Value: 파이프라인명과 setIndex 쌍의 벡터
        std::unordered_map<
            std::vector<VkDescriptorSetLayoutBinding>,
            std::vector<std::tuple<cString, cUint32_t>>,
            BindingHash,
            BindingEqual> bindingCollector;

        for (const auto& [pipelineName, shaders] : this->pipelineShaders)
        {
            // 파이프라인별 바인딩 수집기: setIndex -> bindingIndex -> VkDescriptorSetLayoutBinding
            std::map<cUint32_t, std::map<cUint32_t, VkDescriptorSetLayoutBinding>> pipelineBindingCollector;
            collectPerPipelineBindings(pipelineName, pipelineBindingCollector);

            // setIndex별로 LayoutInfo 생성
            for (const auto& [setIndex, bindingsMap] : pipelineBindingCollector)
            {
                if (bindingsMap.empty())
                {
                    continue;
                }

                // 레이아웃 정보 생성
                std::vector< VkDescriptorSetLayoutBinding> bindings;

                bindings.reserve(bindingsMap.size());
                // map을 vector로 변환
                for (const auto& [bindingIndex, layoutBinding] : bindingsMap)
                {
                    bindings.push_back(layoutBinding);
                }

                // stageFlags를 제외한 나머지 속성이 동일하면 같은 레이아웃으로 간주
                std::vector<VkDescriptorSetLayoutBinding> normalizedBindings = bindings;
                VkShaderStageFlags accumulatedStageFlags = 0;

                // 모든 바인딩의 stageFlags를 누적
                for (const auto& binding : bindings) {
                    accumulatedStageFlags |= binding.stageFlags;
                }

                // 모든 바인딩의 stageFlags를 0으로 설정하여 비교
                for (auto& binding : normalizedBindings) {
                    binding.stageFlags = 0;
                }

                // try_emplace는 C++17부터 지원되는 함수로, 맵에 키가 없을 때만 새로 삽입
                auto [it, inserted] = bindingCollector.try_emplace(
                    normalizedBindings,
                    std::vector<std::tuple<cString, cUint32_t>>{ std::make_tuple(pipelineName, setIndex) });

                // 새로운 레이아웃이 추가된 경우
                if (!inserted) {
                    for (size_t i = 0; i < it->first.size(); i++)
                    {
                        accumulatedStageFlags |= it->first[i].stageFlags;
                    }
                    // 이미 존재하는 레이아웃인 경우, 파이프라인과 setIndex 추가
                    it->second.emplace_back(pipelineName, setIndex);
                }

                // 원본 바인딩 벡터의 stageFlags를 누적된 값으로 업데이트
                auto& keyBindings = const_cast<std::vector<VkDescriptorSetLayoutBinding>&>(it->first);
                for (auto& binding : keyBindings) {
                    binding.stageFlags = accumulatedStageFlags;
                }
            }
        }

        this->layoutInfos.clear();
        this->layoutInfos.reserve(bindingCollector.size());

        // 수집된 레이아웃 정보를 LayoutInfo 구조체로 변환하여 저장
        for (const auto& [bindings, pipelineinfo] : bindingCollector) {
            this->layoutInfos.emplace_back(LayoutInfo{ bindings, std::move(pipelineinfo) });
        }

    }
    
    void VKShaderManager::collectPerPipelineBindings(
        const cString& pipelineName,
        std::map<cUint32_t, std::map<cUint32_t, VkDescriptorSetLayoutBinding>>& bindingCollector) const
    {
        // 파이프라인에 속한 모든 쉐이더의 바인딩 정보를 수집
        const auto& shaders = this->pipelineShaders.at(pipelineName);

        // 각 쉐이더의 바인딩 정보를 순회하며 수집
        for (const auto& shader : shaders) {

            // Reflect 모듈에서 바인딩 정보 추출
            const auto& reflectModule = shader.reflectModule;

            // 각 바인딩 정보를 순회하며 수집
            for (cUint32_t i = 0; i < reflectModule.descriptor_binding_count; ++i) {

                // 바인딩 정보 추출
                const SpvReflectDescriptorBinding* binding = &reflectModule.descriptor_bindings[i];

                if (!binding->name)
                {
                    PRINT_TO_LOGGER("Binding name is empty in shader");
                    continue;
                }

                cUint32_t setIndex = binding->set;
                cUint32_t bindingIndex = binding->binding;

                // 레이아웃 바인딩 생성
                // 만약, 이미 해당 set과 binding이 존재한다면 stageFlags만 업데이트
                // try_emplace는 C++17부터 지원되는 함수로, 맵에 키가 없을 때만 새로 삽입
                // 반환값은 삽입된 요소의 반복자와 삽입 여부를 나타내는 불리언 값
                auto [bindingIt, inserted] = bindingCollector[setIndex].try_emplace(bindingIndex);

                if (inserted) {
                    // 새로운 바인딩이 추가된 경우
                    bindingIt->second = createLayoutBindingFromReflect(binding, static_cast<VkShaderStageFlagBits>(shader.stage));
                }
                else {
                    // 이미 존재하는 바인딩인 경우, stageFlags 업데이트
                    bindingIt->second.stageFlags |= static_cast<VkShaderStageFlagBits>(shader.stage);
                }

            }
        }

    }

    VkDescriptorSetLayoutBinding VKShaderManager::createLayoutBindingFromReflect(const SpvReflectDescriptorBinding* binding, VkShaderStageFlagBits shaderStage) const
    {
        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding->binding;
        layoutBinding.descriptorType = static_cast<VkDescriptorType>(binding->descriptor_type);

        // 만약, SAMPLED_IMAGE 타입이라면 COMBINED_IMAGE_SAMPLER로 변경
        // Vulkan에서는 이미지와 샘플러를 별도로 바인딩하는 경우가 드물기 때문
        if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
        {
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }

        layoutBinding.descriptorCount = binding->count;
        layoutBinding.stageFlags = static_cast<VkShaderStageFlagBits>(shaderStage);
        layoutBinding.pImmutableSamplers = nullptr; // Immutable samplers are not used in this example

        return layoutBinding;
    }

    void VKShaderManager::cleanup()
    {
        for (auto& [pipelineName, shaders] : this->pipelineShaders) {
            for (auto& shader : shaders) {
                shader.cleanup();
            }
        }
        this->pipelineShaders.clear();
        this->layoutInfos.clear();
    }
    std::vector<VkPipelineShaderStageCreateInfo> VKShaderManager::createPipelineShaderStageCIs(cString pipelineName) const
    {
        const auto& shaders = pipelineShaders.at(pipelineName);
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        for (const auto& shader : shaders) {

            VkPipelineShaderStageCreateInfo shaderStageCI{};
            shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCI.stage = shader.stage; // ex: VK_SHADER_STAGE_VERTEX_BIT
            shaderStageCI.module = shader.module;
            shaderStageCI.pName = shader.reflectModule.entry_point_name; // ex: "main"
            shaderStageCI.pSpecializationInfo = nullptr;                  // 필요하면 추가

            shaderStages.push_back(shaderStageCI);
        }
        return shaderStages;
    }
    std::vector<VkVertexInputAttributeDescription> VKShaderManager::createVertexInputAttrDesc(cString pipelineName) const
    {

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        for (const auto& shader : pipelineShaders.at(pipelineName)) {
            if (shader.stage == VK_SHADER_STAGE_VERTEX_BIT) {
                attributeDescriptions = shader.makeVertexInputAttributeDescriptions();
            }
        }

        if (attributeDescriptions.empty())
        {
            EXIT_TO_LOGGER("No vertex shader found in the shader manager.");
        }

        return attributeDescriptions;
    }
    
}