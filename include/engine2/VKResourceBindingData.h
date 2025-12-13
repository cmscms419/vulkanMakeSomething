#ifndef VK_RESOURCEBINDINGDATA_H_
#define VK_RESOURCEBINDINGDATA_H_

#include "common.h"
#include "struct.h"
#include "log.h"
#include "VKbarrier2.h"

namespace vkengine {


    struct VKResourceBinding {

        friend class VKImage2D;
        friend class VKBaseBuffer2;
        friend class DescriptorSetHander;
        
        void update() {
            if (buffer != VK_NULL_HANDLE)
            {
                bufferInfo.buffer = buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = bufferSize;
            }
            else if (image && sampler)
            {
                descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                imageInfo.imageView = imageView;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.sampler = sampler;
            }
            else if (image)
            {
                descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                imageInfo.imageView = imageView;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            else
            {
                vkengine::Log::EXIT_TO_LOGGER("Neither image is ready");
            }
        }

        void setSampler(VkSampler sampler) {
            this->sampler = sampler;
            update();
        }
        VKBarrierHelper& getBarrierHelper() { return barrierHelper; }
    
    private:
        VkImage image{ VK_NULL_HANDLE };
        VkImageView imageView{ VK_NULL_HANDLE };
        VkImageLayout imageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
        VkSampler sampler{ VK_NULL_HANDLE };

        VkDescriptorType descriptorType{};
        cUint32_t descriptorCount{};
        VkShaderStageFlags stageFlags{};
        
        VkDescriptorImageInfo imageInfo{};
        VkDescriptorBufferInfo bufferInfo{};
        VkBufferView texelBufferView = { VK_NULL_HANDLE };

        VkBuffer buffer{ VK_NULL_HANDLE };
        VkDeviceSize bufferSize{ 0 };

        VKBarrierHelper barrierHelper;
    };

}



#endif // !VK_RESOURCEBINDINGDATA_H_
