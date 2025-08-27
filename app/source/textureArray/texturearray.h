#ifndef INCLUDE_SOURCE_TEXTURECUBE_H
#define INCLUDE_SOURCE_TEXTURECUBE_H

#include "VKengine.h"
#include "VKtexture.h"
#include "Camera.h"

namespace vkengine
{

    class TextureArrayEngine : public VulkanEngine
    {
    public:
        TextureArrayEngine(std::string root_path);
        ~TextureArrayEngine();

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

        VertexBuffer VKvertexBuffer{};
        IndexBuffer VKindexBuffer{};
        Vk2DArrayTexture textureArray = {};

        std::vector<subUniformBuffer> VKuniformBuffer = {};

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;                      // 그래픽스 파이프라인 -> 그래픽스 파이프라인을 생성
        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };
    };
}

#endif // INCLUDE_SOURCE_TEXTURECUBE_H