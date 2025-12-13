#ifndef INCLUDE_VK_DESCRIPTOR_MANAGER_s_H_
#define INCLUDE_VK_DESCRIPTOR_MANAGER_s_H_

// 해당 코드는 Real-Time Rendering with Vulkan 이라는 강의에서 발췌한 코드입니다.
// 해당 강의 주소 : https://www.honglab.ai/courses/real-time-vulkan

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

        // 스크립트로부터 descriptor pool을 생성한다.
        void createFromScript();

        // 현재 남아있는 capacity로부터 descriptor set을 할당할 수 있는지 확인한다.
        cBool canAllocateFromRemaining(const std::unordered_map<VkDescriptorType, cUint32_t>& requiredTypeCounts,
            cUint32_t requiredNumSets) const;

        // 새로운 descriptor pool을 생성한다.
        void createNewPool(const std::vector<VkDescriptorPoolSize>& typeCounts, cUint32_t maxSets);

        // remaining capacity를 업데이트 한다.
        void updateRemainingCapacity(const std::vector<VkDescriptorSetLayoutBinding>& bindings,
            uint32_t numSets);
        
        // allocated capacity를 업데이트 한다.
        VkDescriptorSet allocateDescriptorSet(const VkDescriptorSetLayout layout);

        // layout들을 생성한다.
        void createLayouts(const std::vector<LayoutInfo>& layoutInfos);

        // 할당된 descriptor 개수 통계를 출력한다.
        void printAllocatedStatistics() const;

        std::vector<VkDescriptorSetLayout> getLayoutsForPipeline(const cString& pipelineName);

        const VkDescriptorSetLayout& getDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

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