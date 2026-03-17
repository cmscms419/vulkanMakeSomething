#include "VKDescriptorManager2.h"

#include <fstream>
#include <sstream>

#include "type.h"
#include "helper.h"
#include "log.h"

using namespace vkengine::Log;
        
static VkDescriptorSetLayout NULL_P = VK_NULL_HANDLE;

namespace vkengine {

    DescriptorManager2::DescriptorManager2(VkDevice& device) : logicaldevice(device) {
        descriptorPools = {};
        layoutsAndInfos = {};
        allocatedTypeCounts = {};
        remainingTypeCounts = {};
        allocatedSets = 0;
        remainingSets = 0;
    }

    const std::vector<VkDescriptorSetLayoutBinding>& DescriptorManager2::layoutToBindings(const VkDescriptorSetLayout& layout)
    {
        for (const auto& [storedLayout, layoutInfo] : layoutsAndInfos)
        {
            if (storedLayout == layout)
            {
                return layoutInfo.bindings;
            }
        }

        EXIT_TO_LOGGER("Error: Descriptor set layout not found in DescriptorManager2.");

        static const std::vector<VkDescriptorSetLayoutBinding> emptyBindings;
        return emptyBindings;

    }

    DescriptorManager2::~DescriptorManager2()
    {
        cleanup();
    }

    void DescriptorManager2::createFromScript()
    {
        std::ifstream file(kScriptFilename);

        if (!file.is_open())
        {
            PRINT_TO_LOGGER("Error: Unable to open descriptor_pools.txt script file.");
            return;
        }

        std::string line;
        std::vector<VkDescriptorPoolSize> poolSizes;
        cUint32_t numSets = 0;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            cString typeStr;
            cUint32_t count;

            if (iss >> typeStr >> count) {
                if (typeStr == "NumSets") {
                    numSets = count;
                }
                else {
                    // Convert string to VkDescriptorType using VulkanTools function
                    VkDescriptorType type = helper::descriptor::stringToDescriptorType(typeStr);

                    if (type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
                        VkDescriptorPoolSize poolSize = helper::descriptor::descriptorPoolSize(type, count);
                        poolSizes.push_back(poolSize);
                    }
                }
            }
        }
        file.close();

        // 만약, 이미 데이터를 가지고 있으면, 기존 데이터를 사용한다.
        if (numSets > 0 && !poolSizes.empty())
        {
            this->createNewPool(poolSizes, numSets);
            PRINT_TO_LOGGER("Created initial pool with %u sets and %u descriptor types", numSets, static_cast<unsigned int>(poolSizes.size()));
        }
    }

    cBool DescriptorManager2::canAllocateFromRemaining(const std::unordered_map<VkDescriptorType, cUint32_t>& requiredTypeCounts, cUint32_t requiredNumSets) const
    {
        if (this->descriptorPools.empty()) return false;

        if (this->remainingSets < requiredNumSets) return false;

        for (auto it = requiredTypeCounts.begin(); it != requiredTypeCounts.end(); ++it) {
            VkDescriptorType type = it->first;
            cUint32_t count = it->second;

            auto remainingIt = this->remainingTypeCounts.find(type);
            if (remainingIt == this->remainingTypeCounts.end() || remainingIt->second < count) {
                return false;
            }
        }

        return true;
    }

    void DescriptorManager2::createNewPool(const std::vector<VkDescriptorPoolSize>& typeCounts, cUint32_t maxSets)
    {
        // Create descriptor pool
        // VKdescriptorPoolSize vector를 받아서 descriptor pool을 생성한다.
        VkDescriptorPoolCreateInfo poolInfo = helper::descriptor::descriptorPoolCreateInfo(typeCounts, maxSets);

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        _VK_CHECK_RESULT_(vkCreateDescriptorPool(logicaldevice, &poolInfo, nullptr, &descriptorPool));

        this->descriptorPools.push_back(descriptorPool);

        this->remainingSets += maxSets;
        for (const auto& typeCount : typeCounts) {
            this->remainingTypeCounts[typeCount.type] += typeCount.descriptorCount;
        }

    }

    void DescriptorManager2::updateRemainingCapacity(const std::vector<VkDescriptorSetLayoutBinding>& bindings, uint32_t numSets)
    {
        this->remainingSets -= numSets;

        for (const auto& binding : bindings) {
            this->remainingTypeCounts[binding.descriptorType] -= binding.descriptorCount * numSets;
        }
    }

    VkDescriptorSet DescriptorManager2::allocateDescriptorSet(const VkDescriptorSetLayout layout)
    {
        // 1. layout에 해당하는 binding 정보를 찾는다.
        const std::vector<VkDescriptorSetLayoutBinding>& bindings = this->layoutToBindings(layout);

        // 2. binding 정보를 바탕으로 필요한 descriptor type과 개수를 계산한다.
        std::unordered_map<VkDescriptorType, cUint32_t> requiredTypeCounts;
        for (const auto& binding : bindings) {
            requiredTypeCounts[binding.descriptorType] += binding.descriptorCount;
        }

        // 3. 남아있는 capacity로부터 할당이 가능한지 확인한다.
        if (!this->canAllocateFromRemaining(requiredTypeCounts, 1)) {
            // 3.1.1 pool 생성을 위해 VKdescriptorPoolSize vector를 만든다.
            std::vector<VkDescriptorPoolSize> poolSizes;
            poolSizes.reserve(requiredTypeCounts.size());

            for (const auto& [type, count] : requiredTypeCounts) {
                VkDescriptorPoolSize poolSize = helper::descriptor::descriptorPoolSize(type, count);
                poolSizes.push_back(poolSize);
            }

            // 3.1.2 새로운 pool을 생성한다.
            this->createNewPool(poolSizes, std::max(1u, this->remainingSets * 2));

            // 3.1.3. 남아있는 capacity로부터 할당이 가능한지 다시 확인한다.
            // if (!this->canAllocateFromRemaining(requiredTypeCounts, 1)) {
            //     EXIT_TO_LOGGER("Error: Unable to allocate descriptor set even after creating a new pool.");
            // }
        }

        // 4. 만약에, 어떤 binding이 descriptor count를 가지고 있는지 확인한다.
        cBool hasVariableDescriptorCount = false;
        cUint32_t variableDescriptorCount = 0;
        cUint32_t highestBinding = 0;

        for (const auto& binding : bindings) {
            if (binding.binding > highestBinding) {
                highestBinding = binding.binding;
            }
        }

        for (const auto& binding : bindings) {
            if (binding.descriptorCount > 1 && binding.binding == highestBinding) {
                hasVariableDescriptorCount = true;
                variableDescriptorCount = binding.descriptorCount;
                break;
            }
        }

        // 5. 마지막 pool를 사용해서 descriptor set을 할당한다.
        VkDescriptorSetAllocateInfo allocInfo = helper::descriptor::descriptorSetAllocateInfo(
            this->descriptorPools.back(), layout, 1);

        // descriptor count가 추가로 들어가는 것이 있으면, 해당 설정을 추가할 수 있는 플러그 추가
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
        if(hasVariableDescriptorCount)
        {
            variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
            variableCountInfo.descriptorSetCount = 1;
            variableCountInfo.pDescriptorCounts = &variableDescriptorCount;
            allocInfo.pNext = &variableCountInfo;

            PRINT_TO_LOGGER("Allocating descriptor set with variable count: %d", variableDescriptorCount);
        }

        VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
        VkResult res = vkAllocateDescriptorSets(this->logicaldevice, &allocInfo, &descriptorSet);

        if (res == VK_ERROR_OUT_OF_POOL_MEMORY)
        {
            EXIT_TO_LOGGER("Error: Out of pool memory when allocating descriptor set.");
        }

        // 5. allocated capacity를 업데이트 한다.
        this->updateRemainingCapacity(bindings, 1);
        this->allocatedSets += 1;

        for (const auto& binding : bindings) {
            this->allocatedTypeCounts[binding.descriptorType] += binding.descriptorCount;
        }

        return descriptorSet;
    }

    void DescriptorManager2::createLayouts(const std::vector<LayoutInfo>& layoutInfos)
    {
        for (const auto& layoutInfo : layoutInfos)
        {
            VkDescriptorSetLayoutCreateInfo layoutCreateInfo = helper::descriptor::descriptorSetLayoutCreateInfo(layoutInfo.bindings);

            // 바인딩 번호를 기준으로 바인딩을 정렬하여 가장 높은 번호를 찾습니다.
            std::vector<VkDescriptorSetLayoutBinding> sortedBindings = layoutInfo.bindings;
            std::sort(sortedBindings.begin(), sortedBindings.end(),
                      [](const VkDescriptorSetLayoutBinding &a, const VkDescriptorSetLayoutBinding &b)
                      {
                          return a.binding < b.binding;
                      });
            for(int i = 0; i < layoutInfo.bindings.size(); i++)
            {
                if (layoutInfo.bindings[i].binding != i){
                    EXIT_TO_LOGGER("binding index mismatch %d vs %d", i, layoutInfo.bindings[i].binding);
                }
            }

            cBool needsPartiallyBound = false;
            cBool needsVariableCount = false;
            uint32_t highestBinding = sortedBindings.empty() ? 0 : sortedBindings.back().binding;

            for (const auto& binding : sortedBindings) {
                if (binding.descriptorCount > 1) {
                    needsPartiallyBound = true;

                    // Variable count ONLY allowed on the highest binding number
                    if (binding.binding == highestBinding) {
                        needsVariableCount = true;
                        PRINT_TO_LOGGER("    Binding %d is variable-length array (count=%d)", 
                            binding.binding, binding.descriptorCount);
                    } else {
                        PRINT_TO_LOGGER("    Binding %d is fixed-length array (count=%d) - not last binding",
                            binding.binding, binding.descriptorCount);
                    }
                }
            }

            layoutCreateInfo.flags = 0;

            // PARTIALLY_BOUND 또는 VARIABLE_DESCRIPTOR_COUNT - 이 항목들은 바인딩 플래그에만 사용됩니다.
            std::vector<VkDescriptorBindingFlags> bindingFlags;
            VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};

            if (needsPartiallyBound || needsVariableCount) {
                bindingFlags.resize(layoutInfo.bindings.size(), 0);

                for (size_t i = 0; i < layoutInfo.bindings.size(); ++i) {
                    if (layoutInfo.bindings[i].descriptorCount > 1) {
                        bindingFlags[i] |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
                        bindingFlags[i] |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

                        // 가장 높은 바인딩 번호에 대해서만 변수 개수 계산
                        if (layoutInfo.bindings[i].binding == highestBinding) {
                            bindingFlags[i] |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
                        }
                    }
                }

                bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
                bindingFlagsInfo.pBindingFlags = bindingFlags.data();
                layoutCreateInfo.pNext = &bindingFlagsInfo;
            }
            VkDescriptorSetLayout layout = VK_NULL_HANDLE;

            _VK_CHECK_RESULT_(vkCreateDescriptorSetLayout(this->logicaldevice, &layoutCreateInfo, nullptr, &layout));

            this->layoutsAndInfos.push_back({ layout, layoutInfo });
        }

        PRINT_TO_LOGGER("Created %zu descriptor set layouts.", layoutInfos.size());
        PRINT_TO_LOGGER("ShaderManager: Created %zu ", layoutInfos.size());
        for (size_t i = 0; i < layoutInfos.size(); ++i) {
            const auto& info = layoutInfos[i];
            const auto& layout = std::get<0>(this->layoutsAndInfos[i]);

            PRINT_TO_LOGGER("  Layout %zu (0x%zx): %zu binding(s), used by:", i,
                reinterpret_cast<uintptr_t>(layout), info.bindings.size());
            for (const auto& [pipelineName, setNumber] : info.pipelineNamesAndSetNumbers) {
                PRINT_TO_LOGGER("    - Pipeline '%s', Set %d", pipelineName.c_str(), setNumber);
            }

            // Print binding details
            for (size_t j = 0; j < info.bindings.size(); ++j) {
                const auto& binding = info.bindings[j];
                PRINT_TO_LOGGER("    Binding %d: type= %s, count= %d, stages= %s", binding.binding,
                    helper::descriptor::descriptorTypeToString(binding.descriptorType).c_str(), binding.descriptorCount,
                    helper::shader::shaderStageFlagsToString(binding.stageFlags).c_str());
            }
        }
    }

    void DescriptorManager2::printAllocatedStatistics() const
    {
        PRINT_TO_LOGGER("Descriptor Manager Allocation Statistics:");

        PRINT_TO_LOGGER("Allocated Descriptor Sets: %u", this->allocatedSets);
        PRINT_TO_LOGGER("Allocated Descriptor Types:");

        if (this->allocatedTypeCounts.empty())
        {
            PRINT_TO_LOGGER("- None");
            return;
        }

        for (const auto& [type, count] : this->allocatedTypeCounts)
        {
            PRINT_TO_LOGGER("- %s: %u", helper::descriptor::descriptorTypeToString(type).c_str(), count);
        }

        PRINT_TO_LOGGER("");
    }

    std::vector<VkDescriptorSetLayout> DescriptorManager2::getLayoutsForPipeline(const cString& pipelineName)
    {
        std::vector<VkDescriptorSetLayout> layouts;

        for (const auto& [layout, info] : this->layoutsAndInfos)
        {
            for (const auto& [pName, setNumber] : info.pipelineNamesAndSetNumbers)
            {
                if (pName == pipelineName)
                {
                    if (layouts.size() < setNumber + 1)
                    {
                        layouts.resize(setNumber + 1, VK_NULL_HANDLE);
                    }
                    layouts[setNumber] = layout;
                }
            }
        }
        return layouts;
    }

    const VkDescriptorSetLayout& DescriptorManager2::getDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    {
        // bindings와 일치하는 layout을 찾는다.
        for (const auto& [storedLayout, layoutInfo] : this->layoutsAndInfos)
        {
            if (BindingEqual{}(layoutInfo.bindings, bindings))
            {
                return storedLayout;  // 값 반환
            }
        }

        return NULL_P;  // 예외 처리 (실제로는 실행되지 않음)
    }

    void DescriptorManager2::cleanup()
    {
        if (!this->descriptorPools.empty())
        {
            std::ofstream file(this->kScriptFilename);

            if (file.is_open())
            {
                file << "NumSets " << this->remainingSets << "";

                // 남아있는 descriptor type과 개수를 스크립트 파일에 기록한다.
                for (const auto& [type, count] : this->remainingTypeCounts)
                {
                    file << helper::descriptor::descriptorTypeToString(type) << " " << count << "";
                }
                file.close();
                PRINT_TO_LOGGER("Saved remaining descriptor pool capacity to %s", this->kScriptFilename.c_str());
            }
            else
            {
                PRINT_TO_LOGGER("Error: Unable to open descriptor_pools.txt script file for writing.");
            }
        }

        for (auto& pool : this->descriptorPools)
        {
            if (pool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(this->logicaldevice, pool, nullptr);
                pool = VK_NULL_HANDLE;
            }
        }

        this->descriptorPools.clear();

        for (auto& [layout, info] : this->layoutsAndInfos)
        {
            if (layout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(this->logicaldevice, layout, nullptr);
                layout = VK_NULL_HANDLE;
            }
        }

        this->layoutsAndInfos.clear();
    }
}