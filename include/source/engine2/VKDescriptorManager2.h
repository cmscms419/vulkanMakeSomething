#ifndef INCLUDE_VK_DESCRIPTOR_MANAGER_s_H_
#define INCLUDE_VK_DESCRIPTOR_MANAGER_s_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

namespace vkengine {
    
    struct setLayoutBinding {

        VkDescriptorSetLayout layout{ VK_NULL_HANDLE }; ///< 디스크립터 세트 레이아웃
        VkDescriptorSetLayoutBinding binding{}; ///< 디스크립터 세트 레이아웃 바인딩 정보
        VkDescriptorSetLayoutCreateInfo layoutInfo{}; ///< 디스크립터 세트 레이아웃 생성 정보

        setLayoutBinding(
            VkDescriptorSetLayoutBinding binding = VkDescriptorSetLayoutBinding{}) :
            binding(binding)
        {};
        void createDescriptorSetLayout(VkDevice device);
        void createInfo(cUint32_t bingingCount = 1);
    };

    struct DescriptorManager2 {

        DescriptorManager2(VkDevice& device) : logicaldevice(device) {};

        VkDescriptorPool pool{ VK_NULL_HANDLE };
        std::unordered_map<cString, setLayoutBinding> setLayouts;

    private:
        VkDevice& logicaldevice;
    };

}

#endif // !INCLUDE_VK_DESCRIPTOR_MANAGER_H_