#ifndef INCLUDE_SKYMAP_H_
#define INCLUDE_SKYMAP_H_

#include "../source/engine/VKengine.h"
#include "../source/engine/helper.h"
#include "../source/engine/Camera.h"
#include "../source/engine/Debug.h"

#include "../common/struct.h"

namespace vkengine
{
    namespace gui {
        class vkGUI;
    }

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
        void createUniformBuffers();

        // Descriptor�� set, pool, layout�� �����ϱ� ���� �Լ���
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();

        // grapics pipeline�� �����ϱ� ���� �Լ�
        void createGraphicsPipeline();

        void updateUniformBuffer(uint32_t currentImage);

        void cleanupSwapcChain();

        VertexBuffer VKvertexBuffer{};
        IndexBuffer VKindexBuffer{};

        std::vector<UniformBuffer> VKuniformBuffer = {};

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;                      // �׷��Ƚ� ���������� -> �׷��Ƚ� ������������ ����
        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };
    };
}

#endif // !INCLUDE_SKYMAP_H_
