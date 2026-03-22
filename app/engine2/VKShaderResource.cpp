#include "VKShaderResource.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{
    void VKShaderResource::update()
    {
        if (buffer)
        {
            bufferInfo.buffer = buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = bufferSize;
        }
        else if (image && sampler)
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
        else
        {
            EXIT_TO_LOGGER("Neither image is ready");
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

    void VKShaderResource::transitionTo(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkAccessFlags2 newAccess, VkPipelineStageFlags2 newStage)
    {
        this->barrierHelper.transitionImageLayout2(
            commandBuffer, this->image, newLayout, newAccess, newStage);
        updateResourceBindingAfterTransition();
    }

    void VKShaderResource::transitionToColorAttachment(VkCommandBuffer commandBuffer)
    {
        transitionTo(commandBuffer,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    void VKShaderResource::transitionToDepthStencilAttachment(VkCommandBuffer commandBuffer)
    {
        transitionTo(commandBuffer,
                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                     VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT);
    }

    void VKShaderResource::transitionToTransferDst(VkCommandBuffer commandBuffer)
    {
        transitionTo(commandBuffer,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_ACCESS_2_TRANSFER_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    }

    void VKShaderResource::transitionToShaderReadOnly(VkCommandBuffer commandBuffer)
    {
        transitionTo(commandBuffer,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_ACCESS_2_SHADER_READ_BIT,
                     VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    }

    void VKShaderResource::transitionToShaderReadWrite(VkCommandBuffer commandBuffer)
    {
        transitionTo(commandBuffer,
                     VK_IMAGE_LAYOUT_GENERAL,
                     VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    }

    void VKShaderResource::transitionToShaderWriteOnly(VkCommandBuffer commandBuffer)
    {
        EXIT_TO_LOGGER("TODO: 아직 만들지 않음");
    }

    void VKShaderResource::transitionToPresent(VkCommandBuffer commandBuffer)
    {
        transitionTo(commandBuffer,
                     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                     VK_ACCESS_2_NONE,
                     VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
    }

    void VKShaderResource::transitionToTransferSrc(VkCommandBuffer commandBuffer)
    {
        transitionTo(commandBuffer,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     VK_ACCESS_2_TRANSFER_READ_BIT,
                     VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    }

    void VKShaderResource::updateResourceBindingAfterTransition()
    {
        VkImageLayout currentLayout = this->barrierHelper.Currentlayout();

        if (currentLayout == VK_IMAGE_LAYOUT_GENERAL)
        {
            this->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            this->imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
        else if (currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            this->descriptorType = (this->sampler != VK_NULL_HANDLE)
                                       ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                                       : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            this->imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        else if (currentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                 currentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            this->descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            this->imageInfo.imageLayout = currentLayout;
        }
        else
        {
            this->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            this->imageInfo.imageLayout = currentLayout;
        }
    }

}
