#include "TextureTable.h"

#include "vkmath.h"
#include "materialData.h"
#include "log.h"

#define MAX_TEXTURE_COUNTER 512


namespace vkengine
{
    TextureTable::TextureTable(Context &ctx) : ctx(ctx)
    {
    }

    TextureTable::TextureTable(TextureTable &&other) noexcept
        : ctx(other.ctx), entriesTextures(std::move(other.entriesTextures))
    {
    }

    TextureTable::~TextureTable()
    {
        this->cleanup();
    }

    void TextureTable::cleanup()
    {
        if (!this->entriesTextures.empty())
        {
            for (auto &texture : this->entriesTextures)
            {
                texture->cleanup();
            }
        }

        entriesTextures.clear();
    }

    void TextureTable::updateBinding(VkDescriptorSetLayoutBinding &binding)
    {
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = MAX_TEXTURE_COUNTER;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = 0; // Will be filled by shader reflection
    }

    void TextureTable::updateWrite(VkWriteDescriptorSet &write)
    {
        static std::vector<VkDescriptorImageInfo> imageInfos;

        imageInfos.clear();
        imageInfos.resize(this->entriesTextures.size());
        for (size_t i = 0; i < this->entriesTextures.size(); ++i)
        {
            if (!this->entriesTextures[i])
            {
                Log::exitToLogger("Texture was not created.");
            }
            
            this->entriesTextures[i]->updateImageInfo(imageInfos[i]);
        }

        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstSet = VK_NULL_HANDLE; // Will be set by DescriptorSet::create()
        write.dstBinding = 0;          // Will be set by DescriptorSet::create()
        write.dstArrayElement = 0;     // Bindless Texture index
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = uint32_t(this->entriesTextures.size());
        write.pBufferInfo = nullptr;
        write.pImageInfo = imageInfos.data();
        write.pTexelBufferView = nullptr; // Not implemented yet
    }

    std::vector<std::unique_ptr<Image2D>> &TextureTable::getTextures()
    {
        return this->entriesTextures;
    }
}
