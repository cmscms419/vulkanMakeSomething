#include "VKDescriptorManager.h"

namespace vkengine {

    void DescriptorManager::createDescriptorPool(VkDescriptorPoolCreateInfo poolInfo)
    {
        _VK_CHECK_RESULT_(vkCreateDescriptorPool(this->logicaldevice, &poolInfo, nullptr, &this->pool));
    }

    void DescriptorManager::createDescriptorSetLayouts(cString name)
    {
        if (this->setLayouts.empty())
        {
            _PRINT_TO_CONSOLE_("DescriptorManager::createDescriptorSetLayouts() : setLayouts is empty.\n");
            return;
        }

        this->setLayouts[name].createDescriptorSetLayout(this->logicaldevice);
    }

    void setLayoutBinding::createDescriptorSetLayout(VkDevice device)
    {
        if (this->layoutInfo.bindingCount == 0)
        {
            _PRINT_TO_CONSOLE_("layout isn't already created.\n");
            return;
        }

        if (this->binding.descriptorCount == 0)
        {
            _PRINT_TO_CONSOLE_("descriptorCount is 0.\n");
            return;
        }

        _VK_CHECK_RESULT_(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &this->layout));
    }

    void setLayoutBinding::createInfo(cUint32_t bingingCount)
    {
        if (bingingCount == 0)
        {
            _PRINT_TO_CONSOLE_("DescriptorSetLayoutBinding::createInfo() : descriptorCount is 0, set to 1.\n");
            return;
        }

        layoutInfo = helper::descriptorSetLayoutCreateInfo2(binding, bingingCount);
    }

}