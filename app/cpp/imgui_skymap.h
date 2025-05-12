#ifndef INCLUDE_SKYMAP_IMGUI_H_
#define INCLUDE_SKYMAP_IMGUI_H_

#include "../source/engine/VKengine.h"
#include "../source/engine/VKtexture.h"

#include "../source/engine/component/Camera.h"
#include "../source/engine/component/object3D.h"
#include "../source/engine/component/VKimgui.h"

namespace vkengine
{

    class imguiEngine : public VulkanEngine
    {
    public:
        imguiEngine(std::string root_path);
        ~imguiEngine();

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
        void createGraphicsPipeline2();

        void updateUniformBuffer(uint32_t currentImage);

        void cleanupSwapcChain();

        // imgui ����
        void initUI();
        vkGUI* vkGUI = nullptr;

        // 3d object
        object::TextureArrayObject3D cubeObject;

        // skybox
        object::SkyBox cubeSkybox;

        std::vector<UniformBuffer> VKuniformBuffer = {};
        std::vector<UniformBuffer> VKuniformBuffer2 = {};

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;                      // �׷��Ƚ� ���������� -> �׷��Ƚ� ������������ ����
        VkPipeline VKCubeMapPipeline = VK_NULL_HANDLE;                       // ť��� ���������� -> ť��� ������������ ����

        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };
    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
