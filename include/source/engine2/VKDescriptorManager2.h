#ifndef INCLUDE_VK_DESCRIPTOR_MANAGER_s_H_
#define INCLUDE_VK_DESCRIPTOR_MANAGER_s_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

namespace vkengine {
    
    struct setLayoutBinding {

        VkDescriptorSetLayout layout{ VK_NULL_HANDLE }; ///< ��ũ���� ��Ʈ ���̾ƿ�
        VkDescriptorSetLayoutBinding binding{}; ///< ��ũ���� ��Ʈ ���̾ƿ� ���ε� ����
        VkDescriptorSetLayoutCreateInfo layoutInfo{}; ///< ��ũ���� ��Ʈ ���̾ƿ� ���� ����

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