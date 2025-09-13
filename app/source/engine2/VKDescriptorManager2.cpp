#include "VKDescriptorManager2.h"

namespace vkengine {

    const std::vector<VkDescriptorSetLayoutBinding>& DescriptorManager2::layoutToBindings(const VkDescriptorSetLayout& layout)
    {
        for (const auto& [storedLayout, layoutInfo] : layoutsAndInfos)
        {
            if (storedLayout == layout)
            {
                return layoutInfo.bindings;
            }
        }

        _EXIT_WITH_MESSAGE_("Error: Descriptor set layout not found in DescriptorManager2.\n");

        static const std::vector<VkDescriptorSetLayoutBinding> emptyBindings;
        return emptyBindings;

    }

    DescriptorManager2::~DescriptorManager2()
    {
        cleanup();
    }

    void DescriptorManager2::createFromScript()
    {
        std::ifstream file(kScriptFilename_);

        if (!file.is_open())
        {
            _PRINT_TO_CONSOLE_("Error: Unable to open descriptor_pools.txt script file.\n");
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
                    VkDescriptorType type = helper::stringToDescriptorType(typeStr);

                    if (type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
                        VkDescriptorPoolSize poolSize = helper::descriptorPoolSize(type, count);
                        poolSizes.push_back(poolSize);
                    }
                }
            }
        }
        file.close();

        // ����, �̹� �����͸� ������ ������, ���� �����͸� ����Ѵ�.
        if (numSets > 0 && !poolSizes.empty())
        {
            this->createNewPool(poolSizes, numSets);
            _PRINT_TO_CONSOLE_("Created initial pool with %u sets and %u descriptor types\n", numSets, static_cast<unsigned int>(poolSizes.size()));
        }
    }

    cBool DescriptorManager2::canAllocateFromRemaining(const std::unordered_map
        <VkDescriptorType, cUint32_t>& requiredTypeCounts, cUint32_t requiredNumSets) const
    {
        if (this->descriptorPools.empty()) return false;

        if (this->remainingSets_ < requiredNumSets) return false;

        for (auto it = requiredTypeCounts.begin(); it != requiredTypeCounts.end(); ++it) {
            VkDescriptorType type = it->first;
            cUint32_t count = it->second;

            auto remainingIt = this->remainingTypeCounts_.find(type);
            if (remainingIt == this->remainingTypeCounts_.end() || remainingIt->second < count) {
                return false;
            }
        }

        return true;
    }

    void DescriptorManager2::createNewPool(const std::vector<VkDescriptorPoolSize>& typeCounts, cUint32_t maxSets)
    {
        // Create descriptor pool
        // VKdescriptorPoolSize vector�� �޾Ƽ� descriptor pool�� �����Ѵ�.
        VkDescriptorPoolCreateInfo poolInfo = helper::descriptorPoolCreateInfo(typeCounts, maxSets);

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        _VK_CHECK_RESULT_(vkCreateDescriptorPool(logicaldevice, &poolInfo, nullptr, &descriptorPool));

        this->descriptorPools.push_back(descriptorPool);

        this->remainingSets_ += maxSets;
        for (const auto& typeCount : typeCounts) {
            this->remainingTypeCounts_[typeCount.type] += typeCount.descriptorCount;
        }

    }

    void DescriptorManager2::updateRemainingCapacity(const std::vector<VkDescriptorSetLayoutBinding>& bindings, uint32_t numSets)
    {
        this->remainingSets_ -= numSets;

        for (const auto& binding : bindings) {
            this->remainingTypeCounts_[binding.descriptorType] -= binding.descriptorCount * numSets;
        }
    }

    VkDescriptorSet DescriptorManager2::allocateDescriptorSet(const VkDescriptorSetLayout layout)
    {
        // 1. layout�� �ش��ϴ� binding ������ ã�´�.
        const std::vector<VkDescriptorSetLayoutBinding>& bindings = this->layoutToBindings(layout);

        // 2. binding ������ �������� �ʿ��� descriptor type�� ������ ����Ѵ�.
        std::unordered_map<VkDescriptorType, cUint32_t> requiredTypeCounts;
        for (const auto& binding : bindings) {
            requiredTypeCounts[binding.descriptorType] += binding.descriptorCount;
        }

        // 3. �����ִ� capacity�κ��� �Ҵ��� �������� Ȯ���Ѵ�.
        if (!this->canAllocateFromRemaining(requiredTypeCounts, 1)) {
            // 3.1.1 pool ������ ���� VKdescriptorPoolSize vector�� �����.
            std::vector<VkDescriptorPoolSize> poolSizes;
            poolSizes.reserve(requiredTypeCounts.size());

            for (const auto& [type, count] : requiredTypeCounts) {
                VkDescriptorPoolSize poolSize = helper::descriptorPoolSize(type, count);
                poolSizes.push_back(poolSize);
            }

            // 3.1.2 ���ο� pool�� �����Ѵ�.
            this->createNewPool(poolSizes, std::max(1u, this->remainingSets_ * 2));

            // 3.1.3. �����ִ� capacity�κ��� �Ҵ��� �������� �ٽ� Ȯ���Ѵ�.
            if (!this->canAllocateFromRemaining(requiredTypeCounts, 1)) {
                _EXIT_WITH_MESSAGE_("Error: Unable to allocate descriptor set even after creating a new pool.\n");
            }
        }

        // 4. ������ pool�� ����ؼ� descriptor set�� �Ҵ��Ѵ�.
        VkDescriptorSetAllocateInfo allocInfo = helper::descriptorSetAllocateInfo(
            this->descriptorPools.back(), layout, 1);

        VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
        VkResult res = vkAllocateDescriptorSets(this->logicaldevice, &allocInfo, &descriptorSet);

        if (res == VK_ERROR_OUT_OF_POOL_MEMORY)
        {
            _EXIT_WITH_MESSAGE_("Error: Out of pool memory when allocating descriptor set.\n");
        }

        // 5. allocated capacity�� ������Ʈ �Ѵ�.
        this->updateRemainingCapacity(bindings, 1);
        this->allocatedSets_ += 1;

        for (const auto& binding : bindings) {
            this->allocatedTypeCounts_[binding.descriptorType] += binding.descriptorCount;
        }

        return descriptorSet;
    }

    void DescriptorManager2::createLayouts(const std::vector<LayoutInfo>& layoutInfos)
    {
        for (const auto& layoutInfo : layoutInfos)
        {
            VkDescriptorSetLayoutCreateInfo layoutCreateInfo = helper::descriptorSetLayoutCreateInfo(layoutInfo.bindings);
            VkDescriptorSetLayout layout = VK_NULL_HANDLE;
            
            _VK_CHECK_RESULT_(vkCreateDescriptorSetLayout(this->logicaldevice, &layoutCreateInfo, nullptr, &layout));
            
            this->layoutsAndInfos.push_back({ layout, layoutInfo });
        }

        _PRINT_TO_CONSOLE_("Created %zu descriptor set layouts.\n", layoutInfos.size());
    }

    void DescriptorManager2::printAllocatedStatistics() const
    {
        _PRINT_TO_CONSOLE_("Descriptor Manager Allocation Statistics:\n");

        _PRINT_TO_CONSOLE_("Allocated Descriptor Sets: %u\n", this->allocatedSets_);
        _PRINT_TO_CONSOLE_("Allocated Descriptor Types:\n");

        if (this->allocatedTypeCounts_.empty())
        {
            _PRINT_TO_CONSOLE_("- None\n");
            return;
        }

        for (const auto& [type, count] : this->allocatedTypeCounts_)
        {
            _PRINT_TO_CONSOLE_("- %s: %u\n", helper::descriptorTypeToString(type).c_str(), count);
        }

        _PRINT_TO_CONSOLE_("\n");
    }

    void DescriptorManager2::cleanup()
    {

        if (!this->descriptorPools.empty())
        {
            std::ofstream file(this->kScriptFilename_);

            if (file.is_open())
            {
                file << "NumSets " << this->remainingSets_ << "\n";

                // �����ִ� descriptor type�� ������ ��ũ��Ʈ ���Ͽ� ����Ѵ�.
                for (const auto& [type, count] : this->remainingTypeCounts_)
                {
                    file << helper::descriptorTypeToString(type) << " " << count << "\n";
                }
                file.close();
                _PRINT_TO_CONSOLE_("Saved remaining descriptor pool capacity to %s\n", this->kScriptFilename_.c_str());
            }
            else
            {
                _PRINT_TO_CONSOLE_("Error: Unable to open descriptor_pools.txt script file for writing.\n");
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