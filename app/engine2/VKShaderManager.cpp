#include "VKShaderManager.h"

#include "log.h"
#include "type.h"
#include "helper.h"

using namespace vkengine::Log;

namespace vkengine
{
    VKShaderManager::VKShaderManager(VKcontext &ctx, cString shaderPathPrefix, const std::initializer_list<std::pair<cString, std::vector<cString>>> &pipelineShaders) : ctx(ctx)
    {
        // shahder 정보를 읽어서 저장
        createFromShaders(shaderPathPrefix, pipelineShaders);

        collectLayoutInfos();

        ctx.getDescriptorManager()->createLayouts(this->layoutInfos);
    }

    void VKShaderManager::createFromShaders(cString path, std::initializer_list<std::pair<cString, std::vector<cString>>> pipelineShaders)
    {
        for (const auto &[pipelineName, shaderFiles] : pipelineShaders)
        {
            std::vector<VKshader> &shaders = this->pipelineShaders[pipelineName];
            shaders.reserve(shaderFiles.size());

            for (cString shaderFile : shaderFiles)
            {
                shaderFile = path + shaderFile;

                if (shaderFile.substr(shaderFile.length() - 4) != ".spv")
                {
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
            BindingEqual>
            bindingCollector;

        for (const auto &[pipelineName, shaders] : this->pipelineShaders)
        {
            // 파이프라인별 바인딩 수집기: setIndex -> bindingIndex -> Bindinginfo
            // std::map<cUint32_t, std::map<cUint32_t, Bindinginfo>> pipelineBindingCollector;
            // std::unordered_map<cString, ShaderResourceLayout> pipelineBindingCollector;

            collectPerPipelineBindings(pipelineName, this->Resourcelayouts);

        }
        
        // setIndex별로 LayoutInfo 생성
        for (const auto &[pipelineName, ShaderResourceLayoutMap] : Resourcelayouts)
        {
            if (ShaderResourceLayoutMap.pipelineName.empty())
            {
                PRINT_TO_LOGGER("Pipeline name is empty in shader resource layout");
                continue;
            }

            if (ShaderResourceLayoutMap.sets.empty())
            {
                PRINT_TO_LOGGER("No sets found in shader resource layout for pipeline: %s", pipelineName.c_str());
                continue;
            }

            for (const auto &[setIndex, descriptorSetLayout] : ShaderResourceLayoutMap.sets)
            {
                if (descriptorSetLayout.bindings.empty())
                {
                    PRINT_TO_LOGGER("No bindings found in descriptor set layout for pipeline: %s, set: %u", pipelineName.c_str(), setIndex);
                    continue;
                }
                
                std::vector<VkDescriptorSetLayoutBinding> bindings;
                bindings.reserve(descriptorSetLayout.bindings.size());

                for (const auto &[bindingIndex, layoutBinding] : descriptorSetLayout.bindings)
                {
                    bindings.push_back(layoutBinding.binding);
                }

                // stageFlags를 제외한 나머지 속성이 동일하면 같은 레이아웃으로 간주
                std::vector<VkDescriptorSetLayoutBinding> normalizedBindings = bindings;
                VkShaderStageFlags accumulatedStageFlags = 0;

                // 모든 바인딩의 stageFlags를 누적
                for (const auto &binding : bindings)
                {
                    accumulatedStageFlags |= binding.stageFlags;
                }

                // 모든 바인딩의 stageFlags를 0으로 설정하여 비교
                for (auto &binding : normalizedBindings)
                {
                    binding.stageFlags = 0;
                }

                // try_emplace는 C++17부터 지원되는 함수로, 맵에 키가 없을 때만 새로 삽입
                auto [it, inserted] = bindingCollector.try_emplace(
                    normalizedBindings,
                    std::vector<std::tuple<cString, cUint32_t>>{std::make_tuple(pipelineName, setIndex)});

                // 새로운 레이아웃이 추가된 경우
                if (!inserted)
                {
                    for (size_t i = 0; i < it->first.size(); i++)
                    {
                        accumulatedStageFlags |= it->first[i].stageFlags;
                    }
                    // 이미 존재하는 레이아웃인 경우, 파이프라인과 setIndex 추가
                    it->second.emplace_back(pipelineName, setIndex);
                }

                // 원본 바인딩 벡터의 stageFlags를 누적된 값으로 업데이트
                auto &keyBindings = const_cast<std::vector<VkDescriptorSetLayoutBinding> &>(it->first);
                for (auto &binding : keyBindings)
                {
                    binding.stageFlags = accumulatedStageFlags;
                }
            }
        
        }

        this->layoutInfos.clear();
        this->layoutInfos.reserve(bindingCollector.size());

        // 수집된 레이아웃 정보를 LayoutInfo 구조체로 변환하여 저장
        for (const auto &[bindings, pipelineinfo] : bindingCollector)
        {
            this->layoutInfos.emplace_back(LayoutInfo{bindings, std::move(pipelineinfo)});
        }
    }

    void VKShaderManager::collectPerPipelineBindings(
        const cString &pipelineName,
        std::unordered_map<cString, ShaderResourceLayout> &bindingCollector) const
    {
        // 파이프라인에 속한 모든 쉐이더의 바인딩 정보를 수집
        const auto &shaders = this->pipelineShaders.at(pipelineName);

        // 각 쉐이더의 바인딩 정보를 순회하며 수집
        for (const auto &shader : shaders)
        {
            // Reflect 모듈에서 바인딩 정보 추출
            // binding
            // vertex input attribute
            // push constant block
            // ... 필요한 정보들을 모두 추출하여 bindingCollector에 저장

            // try_emplace는 C++17부터 지원되는 함수로, 맵에 키가 없을 때만 새로 삽입
            // 반환값은 삽입된 요소의 반복자와 삽입 여부를 나타내는 불리언 값
            auto &[layout, inserted] = bindingCollector.try_emplace(pipelineName, ShaderResourceLayout{}); // TODO: ShaderResourceLayout 구조체 초기화
            ShaderResourceLayout &shaderResourceLayout = layout->second;

            const auto &reflectModule = shader.reflectModule;
            shaderResourceLayout.pipelineName = pipelineName;

            // 각 바인딩 정보를 순회하며 수집
            for (cUint32_t i = 0; i < reflectModule.descriptor_binding_count; ++i)
            {
                // 바인딩 정보 추출
                const SpvReflectDescriptorBinding *binding = &reflectModule.descriptor_bindings[i];

                if (!binding->name)
                {
                    PRINT_TO_LOGGER("Binding name is empty in shader");
                    continue;
                }

                cUint32_t setIndex = binding->set;
                cUint32_t bindingIndex = binding->binding;
                cString bindingName = binding->name;

                // 레이아웃 바인딩 생성
                if (shaderResourceLayout.sets.find(setIndex) == shaderResourceLayout.sets.end())
                {
                    // 해당 set이 아직 존재하지 않는 경우, 새로운 DescriptorSetLayout 생성
                    // 새로운 바인딩이 추가된 경우, setIndex에 해당하는 DescriptorSetLayout 생성
                    VkDescriptorSetLayoutBinding layoutBinding = createLayoutBindingFromReflect(binding, shader.stage);
                    DescriptorSetLayout descriptorSetLayout;

                    descriptorSetLayout.bindings.emplace(bindingIndex, Bindinginfo{bindingName, layoutBinding});
                    shaderResourceLayout.sets.emplace(setIndex, std::move(descriptorSetLayout));
                }
                else
                {
                    // 만약, set이 이미 존재한다면, binding 정보를 업데이트
                    // 이미 존재하는 바인딩인 경우, stageFlags 업데이트
                    // 아니면, 새로운 바인딩 추가
                    if (shaderResourceLayout.sets[setIndex].bindings.size() <= bindingIndex)
                    {
                        // 새로운 바인딩 추가
                        VkDescriptorSetLayoutBinding layoutBinding = createLayoutBindingFromReflect(binding, shader.stage);
                        shaderResourceLayout.sets[setIndex].bindings.emplace(bindingIndex, Bindinginfo{bindingName, layoutBinding});
                    }
                    else
                    {
                        // 이미 존재하는 바인딩인 경우, stageFlags 업데이트
                        shaderResourceLayout.sets[setIndex].bindings[bindingIndex].binding.stageFlags |= static_cast<VkShaderStageFlagBits>(shader.stage);
                    }
                }
            }

            // 각 쉐이더의 push constant 정보 수집
            for (cUint32_t i = 0; i < reflectModule.push_constant_block_count; ++i)
            {
                const SpvReflectBlockVariable *pushConstantBlock = &reflectModule.push_constant_blocks[i];
                // push constant 정보를 수집하여 shaderResourceLayout에 저장

                if (pushConstantBlock->name == nullptr)
                {
                    PRINT_TO_LOGGER("Push constant block name is empty in shader");
                    continue;
                }

                PushConstantinfo pushConstantInfo;
                pushConstantInfo.name = pushConstantBlock->name;
                pushConstantInfo.range.stageFlags = static_cast<VkShaderStageFlags>(shader.stage);
                pushConstantInfo.range.offset = pushConstantBlock->offset;
                pushConstantInfo.range.size = pushConstantBlock->size;

                if (shaderResourceLayout.pushConstant.has_value())
                {
                    // 이미 push constant이 존재하는 경우, stageFlags 업데이트
                    shaderResourceLayout.pushConstant->range.stageFlags |= pushConstantInfo.range.stageFlags;
                }
                else
                {
                    // 새로운 push constant 추가
                    shaderResourceLayout.pushConstant = pushConstantInfo;
                }
            }

            // 각 쉐이더의 vertex input attribute 정보 수집
            if (shader.stage == VK_SHADER_STAGE_VERTEX_BIT)
            {

                shaderResourceLayout.vertexInputs = std::move(shader.makeVertexInputinfo());

                if (shaderResourceLayout.vertexInputs.empty())
                {
                    PRINT_TO_LOGGER("No vertex input attributes found in vertex shader for pipeline: %s", pipelineName.c_str());
                }

            }
        }
    }

    VkDescriptorSetLayoutBinding VKShaderManager::createLayoutBindingFromReflect(const SpvReflectDescriptorBinding *binding, VkShaderStageFlagBits shaderStage) const
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
        for (auto &[pipelineName, shaders] : this->pipelineShaders)
        {
            for (auto &shader : shaders)
            {
                shader.cleanup();
            }
        }
        this->pipelineShaders.clear();
        this->layoutInfos.clear();
        this->Resourcelayouts.clear();
    }
    
    std::vector<VkPipelineShaderStageCreateInfo> VKShaderManager::createPipelineShaderStageCIs(cString pipelineName) const
    {
        const auto &shaders = pipelineShaders.at(pipelineName);
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        for (const auto &shader : shaders)
        {

            VkPipelineShaderStageCreateInfo shaderStageCI{};
            shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCI.stage = shader.stage; // ex: VK_SHADER_STAGE_VERTEX_BIT
            shaderStageCI.module = shader.module;
            shaderStageCI.pName = shader.reflectModule.entry_point_name; // ex: "main"
            shaderStageCI.pSpecializationInfo = nullptr;                 // 필요하면 추가

            shaderStages.push_back(shaderStageCI);
        }
        return shaderStages;
    }
    
    std::vector<VkVertexInputAttributeDescription> VKShaderManager::createVertexInputAttrDesc(cString pipelineName) const
    {

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        for (const auto &shader : pipelineShaders.at(pipelineName))
        {
            if (shader.stage == VK_SHADER_STAGE_VERTEX_BIT)
            {
                attributeDescriptions = shader.makeVertexInputAttributeDescriptions();
            }
        }

        if (attributeDescriptions.empty())
        {
            EXIT_TO_LOGGER("No vertex shader found in the shader manager.");
        }

        return attributeDescriptions;
    }

    VkPushConstantRange VKShaderManager::pushConstantsRange(cString pipelineName)
    {
        const std::vector<VKshader> &shaders = this->pipelineShaders.at(pipelineName);

        // Search through all shaders in the pipeline for push constants
        for (const auto &shader : shaders)
        {
            const auto &reflectModule = shader.reflectModule;

            // Check if this shader has push constants
            if (reflectModule.push_constant_block_count > 0)
            {
                const SpvReflectBlockVariable *pushBlock = &reflectModule.push_constant_blocks[0];

                VkPushConstantRange pushConstantRange{};
                pushConstantRange.stageFlags = static_cast<VkShaderStageFlags>(shader.stage);
                pushConstantRange.offset = 0;
                pushConstantRange.size = pushBlock->size;

                // Accumulate stage flags from other shaders that also use push constants
                for (const auto &otherShader : shaders)
                {
                    if (otherShader.reflectModule.push_constant_block_count > 0)
                    {
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

    const std::unordered_map<cString, std::vector<VKshader>> &VKShaderManager::getPipelineShaders() const
    {
        return pipelineShaders;
    }

    const std::vector<LayoutInfo> &VKShaderManager::getLayoutInfos() const
    {
        return layoutInfos;
    }

    const ShaderResourceLayout &VKShaderManager::getShaderResourceLayout(cString pipelineName) const
    {
        const auto &layouts = Resourcelayouts.at(pipelineName);

        if (layouts.pipelineName.empty())
        {
            EXIT_TO_LOGGER("No shader resource layout found for pipeline: %s", pipelineName.c_str());
        }

        return layouts;
    }
}