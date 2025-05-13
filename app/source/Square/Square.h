#ifndef INCLUDE_SOURCE_APPLICATION_H
#define INCLUDE_SOURCE_APPLICATION_H

#include "VKengine.h"
#include "helper.h"
#include "Camera.h"
#include "Debug.h"

#include "struct.h"

namespace vkengine
{
    namespace object {
        class Camera;
    }

    class square : public VulkanEngine
    {
    public:
        square(std::string root_path);
        ~square();

        virtual void init() override;
        virtual bool prepare() override;
        virtual bool mainLoop() override;
        virtual void cleanup() override;
        virtual void drawFrame() override;
    
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

        std::vector<UniformBuffer> VKuniformBuffer = {};

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;     // 그래픽스 파이프라인 -> 그래픽스 파이프라인을 생성
        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };

    };
}

#endif // INCLUDE_SOURCE_APPLICATION_H