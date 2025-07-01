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
    class PBRModelEngine : public VulkanEngine {
    public:
        PBRModelEngine(std::string root_path);
        ~PBRModelEngine();
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

        void createGraphicsPipeline();
        void createGraphicsPipeline_skymap();
        void initUI();

        void cleanupSwapcChain();

        // imgui ����
        vkGUI* vkGUI = nullptr;

        // skymap ����
        object::SkyBox* skyBox = nullptr; // ��ī�̹ڽ� ������Ʈ
        object::ModelObject* modelObject = nullptr; // �� ������Ʈ

        MaterialBuffer material; // ��Ƽ���� ����
        
        VkPipeline VKSkyMapPipeline = VK_NULL_HANDLE;       // ť��� ���������� -> ť��� ������������ ����
        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;     // �� ������Ʈ ����������

        UniformBufferSkymap uboParams; // ��ī�̹ڽ� ������ ���� �Ķ����
        
        VKDescriptor2* skyboxDescriptor2 = nullptr;         // �� ������Ʈ ��ũ����
        VKDescriptor2* modeltDescriptor2 = nullptr;         // �� ������Ʈ ��ũ����
        
#if 0
        object::ModelObject* vikingRoomObject = nullptr; // �� ������Ʈ2


        cUint selectModel = 0; // ���õ� �� �ε���
        std::vector<cString> modelNames = {}; // �� �̸���
        cString selectModelName = ""; // ���õ� �� �̸�

        cUint materialIndex = 0; // ��Ƽ���� �ε���
#endif

    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
