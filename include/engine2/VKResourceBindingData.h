#ifndef VK_RESOURCEBINDINGDATA_H_
#define VK_RESOURCEBINDINGDATA_H_

#include "VKbarrier2.h"

namespace vkengine {

    struct VKResourceBinding {

        friend class VKImage2D;
        friend class VKBaseBuffer2;
        friend class DescriptorSetHander;
        friend class VKShadowMap;
        
        void update();
        void setSampler(VkSampler sampler);
        VKBarrierHelper& getBarrierHelper();
    
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
