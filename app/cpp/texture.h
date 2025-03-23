#ifndef INCLUDE_SOURCE_TEXTURE_H_
#define INCLUDE_SOURCE_TEXTURE_H_

#include "../source/engine/VKengine.h"
#include "../source/engine/helper.h"
#include "../source/engine/Camera.h"
#include "../source/engine/Debug.h"
#include "../source/engine/VKtexture.h"

#include "../common/struct.h"
#include "../common/macros.h"

namespace vkengine
{

    class TextureEngine : public VulkanEngine
    {
    public:
        TextureEngine(std::string root_path);
        ~TextureEngine();

        virtual void init() override;
        virtual bool prepare() override;
        virtual void cleanup() override;
        virtual void drawFrame() override;
        virtual bool mainLoop() override;
        void update(float dt);
    
    protected:
        virtual bool init_sync_structures() override;
        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex) override; // 커맨드 버퍼 레코드
    private:

        // 각 3d 모델을 생성하기 위한 함수
        void createVertexbuffer();
        void createIndexBuffer();
        void createUniformBuffers();

        // Descriptor의 set, pool, layout을 생성하기 위한 함수들
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();

        // grapics pipeline을 생성하기 위한 함수
        void createGraphicsPipeline();
        
        void updateUniformBuffer(uint32_t currentImage);

        void cleanupSwapcChain();

        VertexBuffer VKvertexBuffer2{}; // 정사각형 -> texture 부착
        IndexBuffer VKindexBuffer{};
        std::vector<UniformBuffer> VKuniformBuffer = {};

        Vk2DTexture testTexture;

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;                      // 그래픽스 파이프라인 -> 그래픽스 파이프라인을 생성
        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };
    };
}

#endif // INCLUDE_SOURCE_TEXTURECUBE_H