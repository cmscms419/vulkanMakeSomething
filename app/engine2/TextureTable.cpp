#include "TextureTable.h"

#include "vkmath.h"
#include "materialData.h"
#include "log.h"


namespace vkengine
{
    TextureTable::TextureTable(Context &ctx) : ctx(ctx)
    {
    }

    TextureTable::TextureTable(TextureTable &&other) noexcept
        : ctx(other.ctx),
          sampler(other.sampler),
          entriesTextures(std::move(other.entriesTextures)),
          imageInfos(std::move(other.imageInfos))
    {
        other.sampler = VK_NULL_HANDLE;
    }

    TextureTable::~TextureTable()
    {
        this->cleanup();
    }

    void TextureTable::cleanup()
    {
        this->entriesTextures.clear();
        this->imageInfos.clear();
        this->sampler = VK_NULL_HANDLE;
    }

    void TextureTable::initialize(VkSampler sampler, cString dummyTexturePath)
    {
        if (!this->entriesTextures.empty())
        {
            Log::exitToLogger("TextureTable::initialize : 이미 초기화되었다.");
        }

        this->sampler = sampler;

        // 0번은 항상 더미(UV 체커)다.
        // 테이블이 비지 않으므로 updateWrite() 의 descriptorCount 가 0 이 될 일이
        // 없고 (Vulkan 이 금지한다), "텍스처 없음" 을 -1 대신 유효한 인덱스로
        // 표현할 수 있다.
        //
        // 흰색 1x1 이 아니라 UV 체커인 이유는, 이 자리가 샘플링되는 상황은 전부
        // 버그이기 때문이다. 흰색이면 곱셈의 항등원이라 조용히 묻히지만 체커는
        // 화면에서 바로 눈에 띈다. 셰이더는 지금도 DUMMY_TEXTURE_INDEX 와
        // 비교해서 이 슬롯을 건너뛴다 (노멀맵에는 어떤 더미도 맞지 않는다).
        //
        // sRGB = true : 색 데이터로 쓰는 이미지다.
        std::unique_ptr<Image2D> dummyTexture = std::make_unique<Image2D>(this->ctx);
        dummyTexture->createTextureFromImage(dummyTexturePath, false, true);
        dummyTexture->setSampler(this->sampler);

        this->entriesTextures.push_back(std::move(dummyTexture));
    }

    cUint32_t TextureTable::add(std::unique_ptr<Image2D> texture)
    {
        if (this->entriesTextures.empty())
        {
            Log::exitToLogger("TextureTable::add : initialize() 를 먼저 불러야 한다.");
        }

        if (!texture)
        {
            Log::exitToLogger("TextureTable::add : 텍스처가 비어 있다.");
        }

        // 레이아웃이 MAX_TEXTURE_COUNT 칸으로 고정이라 그 이상은 들어갈 수 없다.
        // 막지 않으면 vkUpdateDescriptorSets 에서 한도를 넘은 descriptorCount 로
        // 터진다.
        if (this->entriesTextures.size() >= static_cast<size_t>(MAX_TEXTURE_COUNT))
        {
            Log::exitToLogger("TextureTable::add : 텍스처가 한도 %d 개를 넘었다.",
                              MAX_TEXTURE_COUNT);
        }

        texture->setSampler(this->sampler);

        const cUint32_t index = static_cast<cUint32_t>(this->entriesTextures.size());
        this->entriesTextures.push_back(std::move(texture));

        return index;
    }

    Image2D &TextureTable::get(cUint32_t index)
    {
        if (index >= this->entriesTextures.size())
        {
            Log::exitToLogger("TextureTable::get : 인덱스 %u 가 범위(%u) 밖이다.",
                              index, this->getCount());
        }

        return *this->entriesTextures[index];
    }

    void TextureTable::updateBinding(VkDescriptorSetLayoutBinding &binding)
    {
        // binding 번호는 DescriptorSetHandle::create() 가 채운다. 여기서 0 을
        // 의미없어서 제거
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = MAX_TEXTURE_COUNT;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = 0; // Will be filled by shader reflection
    }

    void TextureTable::updateWrite(VkWriteDescriptorSet &write)
    {
        if (this->entriesTextures.empty())
        {
            Log::exitToLogger("TextureTable::updateWrite : initialize() 를 먼저 불러야 한다.");
        }

        this->imageInfos.clear();
        this->imageInfos.resize(this->entriesTextures.size());

        for (size_t i = 0; i < this->entriesTextures.size(); ++i)
        {
            if (!this->entriesTextures[i])
            {
                Log::exitToLogger("Texture was not created.");
            }

            this->entriesTextures[i]->updateImageInfo(this->imageInfos[i]);
        }

        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstSet = VK_NULL_HANDLE; // Will be set by DescriptorSet::create()
        write.dstBinding = 0;          // Will be set by DescriptorSet::create()
        write.dstArrayElement = 0;     // Bindless Texture index
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = static_cast<cUint32_t>(this->entriesTextures.size()); // 더미가 있으므로 최소 1
        write.pBufferInfo = nullptr;
        write.pImageInfo = this->imageInfos.data();
        write.pTexelBufferView = nullptr; // Not implemented yet
    }
}
