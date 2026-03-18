#include "VKShaderResource.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{
    void VKShaderResource::update()
    {
        switch (this->type)
        {
        case shaderResourceType::BUFFER:
            bufferInfo.buffer = buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = bufferSize;
            break;
        case shaderResourceType::IMAGE:
            if (image && sampler)
            {
            descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 이미지와 셈플러가 결합된 형태
            imageInfo.imageView = imageView;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.sampler = sampler;
            }
            else if (image)
            {
                descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; // 이미지와 셈플러가 서로 분리된 형태
                imageInfo.imageView = imageView;
            }
            break;
        case shaderResourceType::DEPTH_IMAGE:
            if (image && sampler)
            {
                descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                imageInfo.imageView = depthStencilView;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.sampler = sampler;
            }
            break;
        case shaderResourceType::NONE:
        default:
            EXIT_TO_LOGGER("Neither image is ready");
            break;
        }
    }

    void VKShaderResource::updateBinding(VkDescriptorSetLayoutBinding &binding)
    {
        PRINT_TO_LOGGER("Nothing updateBinding %s", this->name.c_str());
    }
    
    void VKShaderResource::updateWrite(VkWriteDescriptorSet &write)
    {
        PRINT_TO_LOGGER("Nothing updateWrite %s", this->name.c_str());
    }

    void VKShaderResource::setSampler(VkSampler sampler)
    {
        this->sampler = sampler;
        update();
    }
    VKBarrierHelper &VKShaderResource::getBarrierHelper()
    {
        return this->barrierHelper;
    }

    void VKShaderResource::updateImageInfo(VkDescriptorImageInfo &imageInfo)
    {
        imageInfo.sampler = this->imageInfo.sampler;
        imageInfo.imageView = this->imageInfo.imageView;
        imageInfo.imageLayout = this->imageInfo.imageLayout;
    }
    
    void VKShaderResource::updateBufferInfo(VkDescriptorBufferInfo &bufferInfo)
    {
        bufferInfo.buffer = this->bufferInfo.buffer;
        bufferInfo.offset = this->bufferInfo.offset;
        bufferInfo.range = this->bufferInfo.range;
    }
}
