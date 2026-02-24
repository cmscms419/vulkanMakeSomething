#ifndef INCLUDE_HELPER_DESCRIPTOR_H_
#define INCLUDE_HELPER_DESCRIPTOR_H_

#include "common.h"
#include "vkdevice.h"

// https://github.com/SaschaWillems/Vulkan에서 참고해서 함수 생성

namespace vkengine
{
    namespace helper
    {
        namespace descriptor
        {
            // VkWriteDescriptorSet 구조체를 생성하는 함수들

            // Buffer를 사용하는 경우
            VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, cUint32_t binding, VkDescriptorBufferInfo *bufferInfo, cUint32_t descriptorCount = 1);
            
            // Image를 사용하는 경우
            VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, cUint32_t binding, VkDescriptorImageInfo *imageInfo, cUint32_t descriptorCount = 1);

            VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize> &poolSizes, cUint32_t maxSets);

            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, cUint32_t binding, cUint32_t descriptorCount);

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding> &bindings);

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo2(const VkDescriptorSetLayoutBinding &bindings, cUint32_t bindingCount);

            VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(const VkDescriptorPool &descriptorPool, const VkDescriptorSetLayout &pSetLayouts, cUint32_t descriptorSetCount);

            VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType type, cUint32_t descriptorCount);

            cString descriptorTypeToString(VkDescriptorType type);

            VkDescriptorType stringToDescriptorType(const cString &typeStr);

            const cChar *getDescriptorTypeString(SpvReflectDescriptorType type);

        } // namespace descriptor
    } // namespace helper
} // namespace vkengine

#endif // !INCLUDE_HELPER_DESCRIPTOR_H_