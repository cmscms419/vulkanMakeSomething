#ifndef INCLUDE_SKYMAP_IMGUI_H_
#define INCLUDE_SKYMAP_IMGUI_H_

#include "VKengine.h"
#include "VKtexture.h"

#include "Camera.h"
#include "object3D.h"
#include "VKimgui.h"
#include "VKloadModel.h"

namespace vkengine
{
    class Load3DModelEngine : public VulkanEngine
    {
    public:
        Load3DModelEngine(std::string root_path);

        ~Load3DModelEngine();

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

        void createDescriptorPool2_(uint32_t max_sets);
        void createDescriptorSets2();

        // grapics pipeline�� �����ϱ� ���� �Լ�
        void createGraphicsPipeline();
        void createGraphicsPipeline2();

        void updateUniformBuffer(uint32_t currentImage);
        void updateUniformBuffer2(uint32_t currentImage, glm::mat4 matrix);
        
        void cleanupSwapcChain();

        // imgui ����
        void initUI();
        vkGUI* vkGUI = nullptr;

        // 3d ���� ���� vertex, index buffer
        object::ModelObject modelObject; // viking_room

        // 3d object
        //object::TextureArrayObject3D cubeObject;

        // skybox
        object::SkyBox cubeSkybox;

        std::vector<subUniformBuffer> VKuniformBuffer = {};
        std::vector<subUniformBuffer> VKuniformBuffer2 = {};

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;                      // �׷��Ƚ� ���������� -> �׷��Ƚ� ������������ ����
        VkPipeline VKCubeMapPipeline = VK_NULL_HANDLE;                       // ť��� ���������� -> ť��� ������������ ����

        std::vector<VkDescriptorSet> VKdescriptorLoadModelSets = {};

        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };
    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
