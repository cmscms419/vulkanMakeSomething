#ifndef INCLUDE_SKYMAP_H_
#define INCLUDE_SKYMAP_H_

#include "../source/engine/VKengine.h"
#include "../source/engine/VKtexture.h"
#include "../source/engine/Camera.h"

namespace vkengine
{

    class skycubeEngine : public VulkanEngine
    {
    public:
        skycubeEngine(std::string root_path);
        ~skycubeEngine();

        virtual void init() override;
        virtual bool prepare() override;
        virtual void cleanup() override;
        virtual void drawFrame() override;
        virtual bool mainLoop() override;
        void update(float dt);

    protected:
        virtual bool init_sync_structures() override;
        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex) override; // Ŀ�ǵ� ���� ���ڵ�
    private:

        // �� 3d ���� �����ϱ� ���� �Լ�
        void createVertexbuffer();
        void createIndexBuffer();

        void createVertexbuffer2();
        void createIndexBuffer2();

        void createUniformBuffers();
        void createUniformBuffers2();

        // Descriptor�� set, pool, layout�� �����ϱ� ���� �Լ���
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();

        // grapics pipeline�� �����ϱ� ���� �Լ�
        void createGraphicsPipeline();
        void createGraphicsPipeline2();

        void updateUniformBuffer(uint32_t currentImage);

        void cleanupSwapcChain();

        VertexBuffer VKvertexBuffer{};
        IndexBuffer VKindexBuffer{};

        VertexBuffer VKvertexBuffer2{};
        IndexBuffer VKindexBuffer2{};

        Vk2DTextureArray cubeTextureArray = {};
        VKcubeMap cubeMap = {};

        std::vector<UniformBuffer> VKuniformBuffer = {};
        std::vector<UniformBuffer> VKuniformBuffer2 = {};

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;                      // �׷��Ƚ� ���������� -> �׷��Ƚ� ������������ ����
        VkPipeline VKCubeMapPipeline = VK_NULL_HANDLE;                       // ť��� ���������� -> ť��� ������������ ����
        
        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };
    };
}

#endif // !INCLUDE_SKYMAP_H_
