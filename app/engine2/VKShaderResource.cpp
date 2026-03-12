#include "VKShaderResource.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{
    void VKShaderResource::update()
    {
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
            EXIT_TO_LOGGER("Neither image is ready");
        }
    }
    
    void VKShaderResource::updateBinding(VkDescriptorSetLayoutBinding &binding)
    {
        binding.descriptorType = this->descriptorType;
        binding.descriptorCount = 1;
        binding.stageFlags = 0;
    }

    void VKShaderResource::updateWrite(VkWriteDescriptorSet &write)
    {
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = this->descriptorType;
        write.descriptorCount = 1;
        if (buffer != VK_NULL_HANDLE)
            write.pBufferInfo = &this->bufferInfo;
        else
            write.pImageInfo = &this->imageInfo;
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
}
