#ifndef INCLUDE_SKYMAP_IMGUI_H_
#define INCLUDE_SKYMAP_IMGUI_H_

#include "VKengine.h"
#include "VKtexture.h"

#include "Camera.h"
#include "object3D.h"
#include "VKimgui.h"
#include "VKloadModel.h"
#include "VKmodelDescriptor.h"
#include "VKSkymapModelDescriptor.h"

namespace vkengine
{
    class DescriptorCodeUpdateEngine : public VulkanEngine
    {
    public:
        DescriptorCodeUpdateEngine(std::string root_path);

        ~DescriptorCodeUpdateEngine();

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

        void createDescriptor();

        // grapics pipeline�� �����ϱ� ���� �Լ�
        void createGraphicsPipeline();
        void createGraphicsPipeline2();
        void createGraphicsPipeline_skymap();

        void cleanupSwapcChain();

        // imgui ����
        void initUI();
        vkGUI* vkGUI = nullptr;

        VK3DModelDescriptor* modelObjectDescriptor = nullptr;
        VKSkyMapModelDescriptor* skyMapModelDescriptor = nullptr;

        // 3d ���� ���� vertex, index buffer
        object::ModelObject* modelObject;  // viking_room
        object::ModelObject* modelObject2; // viking_room

        glm::mat4 worldMatrix = glm::mat4(1.0f); // ���� ��Ʈ����

        // skybox
        object::SkyBox* cubeSkybox;

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;                      // �׷��Ƚ� ���������� -> �׷��Ƚ� ������������ ����
        VkPipeline VKgraphicsPipeline2 = VK_NULL_HANDLE;                     // �׷��Ƚ� ����������2 -> cube ������������ ����
        VkPipeline VKSkyMapPipeline = VK_NULL_HANDLE;                       // ť��� ���������� -> ť��� ������������ ����

        std::vector<VkDescriptorSet> VKdescriptorLoadModelSets = {};

        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };
    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
