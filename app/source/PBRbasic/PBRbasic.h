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
    struct subData
    {
        cVec4 camPos = cVec4(0.0f);
        cVec4 lightPos = cVec4(0.0f);
        cVec4 objectPos = cVec4(0.0f);
        cBool useTexture = false; // ���� Ȱ��ȭ ����
    };

    struct subUinform  : public VkBufferObject
    {
        subData subUniform = {};
    };

    class PBRbasuceEngube : public VulkanEngine {
    public:
        PBRbasuceEngube(std::string root_path);
        ~PBRbasuceEngube();
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

        object::SkyBox* skyBox = nullptr; // ��ī�̹ڽ�
        object::ModelObject* modelObject = nullptr; // �� ������Ʈ
        object::ModelObject* vikingRoomObject = nullptr; // �� ������Ʈ2

        VkPipeline VKSkyMapPipeline = VK_NULL_HANDLE;       // ť��� ���������� -> ť��� ������������ ����
        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;     // �� ������Ʈ ����������

        VKDescriptor2* modeltDescriptor2 = nullptr;         // �� ������Ʈ ��ũ����
        VKDescriptor2* skyboxDescriptor2 = nullptr;         // �� ������Ʈ ��ũ����

        cUint selectModel = 0; // ���õ� �� �ε���
        std::vector<cString> modelNames = { "Sphere", "Viking Room" }; // �� �̸���
        cString selectModelName = "Sphere"; // ���õ� �� �̸�

        MaterialBuffer material; // ��Ƽ���� ����
        subUinform subUniform; // ���� ������ ����
        cUint materialIndex = 0; // ��Ƽ���� �ε���
    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
