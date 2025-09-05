#ifndef INCLUDE_VK_DESCRIPTOR_MANAGER_H_
#define INCLUDE_VK_DESCRIPTOR_MANAGER_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKengine.h"
#include "VKbuffer.h"
#include "VKtexture.h"

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
    
    struct DescriptorManager {

        DescriptorManager(VkDevice device = VK_NULL_HANDLE) : logicaldevice(device) {};

        void createDescriptorPool(VkDescriptorPoolCreateInfo poolInfo);
        void createDescriptorSetLayouts(cString name);
        
        VkDescriptorPool pool{ VK_NULL_HANDLE };
        std::unordered_map<cString, setLayoutBinding> setLayouts;

    private:
        VkDevice logicaldevice{ VK_NULL_HANDLE };
    };

}

#endif // !INCLUDE_VK_DESCRIPTOR_MANAGER_H_