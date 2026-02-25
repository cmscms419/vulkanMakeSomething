#include "VKResourceBindingData.h"
#include "log.h"

namespace vkengine
{
    void VKResourceBinding::update()
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
            vkengine::Log::EXIT_TO_LOGGER("Neither image is ready");
        }
    }

    void VKResourceBinding::setSampler(VkSampler sampler)
    {
        this->sampler = sampler;
        update();
    }

    VKBarrierHelper &VKResourceBinding::getBarrierHelper()
    {
        return barrierHelper;
    }
}