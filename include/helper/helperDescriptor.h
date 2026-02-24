#ifndef INCLUDE_HELPER_DESCRIPTOR_H_
#define INCLUDE_HELPER_DESCRIPTOR_H_

#include <set>
#include <fstream>

#include "common.h"
#include "vkdevice.h"
#include "log.h"

// https://github.com/SaschaWillems/Vulkan에서 참고해서 함수 생성

namespace vkengine
{
    namespace helper
    {
        namespace descriptor
        {
            // VkWriteDescriptorSet 구조체를 생성하는 함수들

            // Buffer를 사용하는 경우
            inline VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, cUint32_t binding, VkDescriptorBufferInfo *bufferInfo, cUint32_t descriptorCount = 1)
            {
                VkWriteDescriptorSet writeDescriptorSet{};
                writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstSet = dstSet;
                writeDescriptorSet.descriptorType = type;
                writeDescriptorSet.dstBinding = binding;
                writeDescriptorSet.pBufferInfo = bufferInfo;
                writeDescriptorSet.descriptorCount = descriptorCount;
                return writeDescriptorSet;
            }

            // Image를 사용하는 경우
            inline VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, cUint32_t binding, VkDescriptorImageInfo *imageInfo, cUint32_t descriptorCount = 1)
            {
                VkWriteDescriptorSet writeDescriptorSet{};
                writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstSet = dstSet;
                writeDescriptorSet.descriptorType = type;
                writeDescriptorSet.dstBinding = binding;
                writeDescriptorSet.pImageInfo = imageInfo;
                writeDescriptorSet.descriptorCount = descriptorCount;
                return writeDescriptorSet;
            }

            inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize> &poolSizes, cUint32_t maxSets)
            {
                VkDescriptorPoolCreateInfo descriptorPoolInfo{};
                descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                descriptorPoolInfo.poolSizeCount = static_cast<cUint32_t>(poolSizes.size());
                descriptorPoolInfo.pPoolSizes = poolSizes.data();
                descriptorPoolInfo.maxSets = maxSets;
                return descriptorPoolInfo;
            }

            inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, cUint32_t binding, cUint32_t descriptorCount = 1)
            {
                VkDescriptorSetLayoutBinding setLayoutBinding{};
                setLayoutBinding.descriptorType = type;
                setLayoutBinding.stageFlags = stageFlags;
                setLayoutBinding.binding = binding;
                setLayoutBinding.descriptorCount = descriptorCount;
                return setLayoutBinding;
            }

            inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding> &bindings)
            {
                VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
                descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                descriptorSetLayoutCreateInfo.pBindings = bindings.data();
                descriptorSetLayoutCreateInfo.bindingCount = static_cast<cUint32_t>(bindings.size());
                return descriptorSetLayoutCreateInfo;
            }

            inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo2(const VkDescriptorSetLayoutBinding &bindings, cUint32_t bindingCount = 1)
            {
                VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
                descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                descriptorSetLayoutCreateInfo.pBindings = &bindings;
                descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
                return descriptorSetLayoutCreateInfo;
            }

            inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(const VkDescriptorPool &descriptorPool, const VkDescriptorSetLayout &pSetLayouts, cUint32_t descriptorSetCount = 1)
            {
                VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
                descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                descriptorSetAllocateInfo.descriptorPool = descriptorPool;
                descriptorSetAllocateInfo.pSetLayouts = &pSetLayouts;
                descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;

                return descriptorSetAllocateInfo;
            }

            inline VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType type, cUint32_t descriptorCount)
            {
                VkDescriptorPoolSize descriptorPoolSize{};
                descriptorPoolSize.type = type;
                descriptorPoolSize.descriptorCount = descriptorCount;
                return descriptorPoolSize;
            }

            cString descriptorTypeToString(VkDescriptorType type);

            VkDescriptorType stringToDescriptorType(const cString &typeStr);

            const cChar *getDescriptorTypeString(SpvReflectDescriptorType type);

        } // namespace descriptor
    } // namespace helper
} // namespace vkengine

#endif // !INCLUDE_HELPER_DESCRIPTOR_H_