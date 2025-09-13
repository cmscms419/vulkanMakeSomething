#ifndef INCLUDE_VK_DESCRIPTOR_MANAGER_s_H_
#define INCLUDE_VK_DESCRIPTOR_MANAGER_s_H_

// �ش� �ڵ�� Real-Time Rendering with Vulkan �̶�� ���ǿ��� ������ �ڵ��Դϴ�.
// �ش� ���� �ּ� : https://www.honglab.ai/courses/real-time-vulkan

#include "common.h"
#include "struct.h"
#include "log.h"
#include "helper.h"

#include <fstream>
#include <sstream>

namespace vkengine {

    struct LayoutInfo
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings{};
        std::vector<std::tuple<cString, cUint32_t>> pipelineNamesAndSetNumbers;
    };
    
    struct DescriptorManager2 {

        const std::vector<VkDescriptorSetLayoutBinding>&
            layoutToBindings(const VkDescriptorSetLayout& layout);

        DescriptorManager2(VkDevice& device) : logicaldevice(device) {};
        ~DescriptorManager2();

        // ��ũ��Ʈ�κ��� descriptor pool�� �����Ѵ�.
        void createFromScript();

        // ���� �����ִ� capacity�κ��� descriptor set�� �Ҵ��� �� �ִ��� Ȯ���Ѵ�.
        cBool canAllocateFromRemaining(const std::unordered_map<VkDescriptorType, cUint32_t>& requiredTypeCounts,
            cUint32_t requiredNumSets) const;

        // ���ο� descriptor pool�� �����Ѵ�.
        void createNewPool(const std::vector<VkDescriptorPoolSize>& typeCounts, cUint32_t maxSets);

        // remaining capacity�� ������Ʈ �Ѵ�.
        void updateRemainingCapacity(const std::vector<VkDescriptorSetLayoutBinding>& bindings,
            uint32_t numSets);
        
        // allocated capacity�� ������Ʈ �Ѵ�.
        VkDescriptorSet allocateDescriptorSet(const VkDescriptorSetLayout layout);

        // layout���� �����Ѵ�.
        void createLayouts(const std::vector<LayoutInfo>& layoutInfos);

        // �Ҵ�� descriptor ���� ��踦 ����Ѵ�.
        void printAllocatedStatistics() const;


    private:
        void cleanup();

        VkDevice& logicaldevice;

        // Descriptor Pools
        std::vector<VkDescriptorPool> descriptorPools{};

        // Layouts and their bindings
        std::vector<std::tuple<VkDescriptorSetLayout, LayoutInfo>> layoutsAndInfos{};

        // Usage tracking
        std::unordered_map<VkDescriptorType, uint32_t> allocatedTypeCounts_{};
        std::unordered_map<VkDescriptorType, uint32_t> remainingTypeCounts_{};
        uint32_t allocatedSets_ = 0;
        uint32_t remainingSets_ = 0;

        const cString kScriptFilename_ = "DescriptorPoolSize.txt";
    };

}

#endif // !INCLUDE_VK_DESCRIPTOR_MANAGER_H_