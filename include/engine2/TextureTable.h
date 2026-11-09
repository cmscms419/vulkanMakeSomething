#ifndef VK_RESOURCE_TABLE_H_
#define VK_RESOURCE_TABLE_H_

#include "common.h"
#include "macros.h"
#include "Context.h"
#include "Image2D.h"
#include "ShaderResource.h"

#include <memory>
#include <vector>

namespace vkengine
{

    // 텍스처를 전부 소유하는 테이블. 바깥에서는 인덱스로만 접근한다.
    //
    //   0번      흰색 1x1 더미. initialize() 가 만든다.
    //            덕분에 테이블이 절대 비지 않는다.
    //   1번부터  add() 로 넣은 실제 텍스처.
    //
    // 인덱스 상수는 include/common/macros.h 의 DUMMY_TEXTURE_INDEX /
    // MAX_TEXTURE_COUNT 이고, 셰이더 쪽 같은 값은 shader/materialCommon.glsl 에 있다.
    //
    // 내부 vector 를 밖으로 내주지 않는다. 인덱스를 발급하는 책임이 테이블에만
    // 있어야 번호가 겹치지 않기 때문이다.
    class TextureTable : public ShaderResource
    {
    public:
        TextureTable(Context &ctx);
        TextureTable(TextureTable &&other) noexcept;
        ~TextureTable();
        TextureTable(const TextureTable &) = delete;
        TextureTable &operator=(const TextureTable &) = delete;
        TextureTable &operator=(TextureTable &&) = delete;

        virtual void updateBinding(VkDescriptorSetLayoutBinding &binding) override;
        virtual void updateWrite(VkWriteDescriptorSet &write) override;

        // 더미 0번을 만들고 샘플러를 보관한다. add() 보다 먼저 불러야 한다.
        // dummyTexturePath 는 RootPath 까지 포함한 완성된 경로여야 한다
        // (파일명은 data.h 의 DUMMY_TEXTURE_PATH).
        void initialize(VkSampler sampler, cString dummyTexturePath);

        // 소유권을 받아 전역 인덱스를 발급한다. 보관 중인 샘플러를 적용한다.
        cUint32_t add(std::unique_ptr<Image2D> texture);

        // 인덱스로 꺼낸다. 범위 밖이면 멈춘다.
        Image2D &get(cUint32_t index);

        cUint32_t getCount() const { return static_cast<cUint32_t>(this->entriesTextures.size()); }
        cBool isInitialized() const { return !this->entriesTextures.empty(); }

    private:
        Context &ctx;

        VkSampler sampler{VK_NULL_HANDLE};

        std::vector<std::unique_ptr<Image2D>> entriesTextures;

        // 예전에는 updateWrite() 안의 함수 지역 static 이었다. 그러면 테이블이
        // 둘 이상일 때 서로의 데이터를 덮어쓴다. 테이블마다 따로 들고 있어야 한다.
        std::vector<VkDescriptorImageInfo> imageInfos;

        void cleanup() override;
    };

}

#endif // VK_RESOURCE_TABLE_H_
