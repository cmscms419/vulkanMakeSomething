#ifndef VK_RESOURCE_TABLE_H_
#define VK_RESOURCE_TABLE_H_

#include "common.h"
#include "VKContext.h"
#include "VKImage2D.h"
#include "VKShaderResource.h"

#include <memory>
#include <vector>

namespace vkengine
{

    // 바인딩 하려는 텍스처를 관리하는 테이블
    // 바인딩을 할 때, 인덱스가 겹치지 않고 관리할 수 있는 테이블
    class VKtexturesTable : public VKShaderResource
    {
    public:
        VKtexturesTable(VKcontext &ctx);
        VKtexturesTable(VKtexturesTable &&other) noexcept;
        ~VKtexturesTable();
        VKtexturesTable(const VKtexturesTable &) = delete;
        VKtexturesTable &operator=(const VKtexturesTable &) = delete;
        VKtexturesTable &operator=(VKtexturesTable &&) = delete;

        virtual void updateBinding(VkDescriptorSetLayoutBinding &binding) override;
        virtual void updateWrite(VkWriteDescriptorSet &write) override;

        std::vector<std::unique_ptr<VKImage2D>> &getTextures();

        cUint64_t count() const;
        cUint64_t getTextureCount() const { return static_cast<cUint64_t>(entriesTextures.size()); }

    private:
        VKcontext &ctx;
        std::vector<std::unique_ptr<VKImage2D>> entriesTextures;

        void cleanup() override;
    };

}

#endif // VK_RESOURCE_TABLE_H_