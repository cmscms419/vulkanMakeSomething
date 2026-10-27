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
            VkWriteDescriptorSet makeWriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, cUint32_t binding, VkDescriptorBufferInfo *bufferInfo, cUint32_t descriptorCount = 1);
            
            // Image를 사용하는 경우
            VkWriteDescriptorSet makeWriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, cUint32_t binding, VkDescriptorImageInfo *imageInfo, cUint32_t descriptorCount = 1);

            VkDescriptorPoolCreateInfo makeDescriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize> &poolSizes, cUint32_t maxSets);

            VkDescriptorSetLayoutBinding makeDescriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, cUint32_t binding, cUint32_t descriptorCount);

            VkDescriptorSetLayoutCreateInfo makeDescriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding> &bindings);

            VkDescriptorSetLayoutCreateInfo makeDescriptorSetLayoutCreateInfo2(const VkDescriptorSetLayoutBinding &bindings, cUint32_t bindingCount);

            VkDescriptorSetAllocateInfo makeDescriptorSetAllocateInfo(const VkDescriptorPool &descriptorPool, const VkDescriptorSetLayout &pSetLayouts, cUint32_t descriptorSetCount);

            VkDescriptorPoolSize makeDescriptorPoolSize(VkDescriptorType type, cUint32_t descriptorCount);

            cString convertDescriptorTypeToString(VkDescriptorType type);

            VkDescriptorType convertStringToDescriptorType(const cString &typeStr);

            const cChar *getDescriptorTypeString(SpvReflectDescriptorType type);

        } // namespace descriptor
    } // namespace helper
} // namespace vkengine

#endif // !INCLUDE_HELPER_DESCRIPTOR_H_
