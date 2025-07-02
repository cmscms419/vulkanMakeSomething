#ifndef INCLUDE_IBLPBR_H_
#define INCLUDE_IBLPBR_H_

#include "VKengine.h"
#include "VKtexture.h"

#include "object3D.h"
#include "VKimgui.h"
#include "VKloadModel.h"
#include "VKmodelDescriptor.h"
#include "VKSkymapModelDescriptor.h"

#include "VKnormalRander.h"

namespace vkengine
{
    struct subData
    {
        cVec3 camPos = cVec3(0.0f);
        char _padding0[4];
        cVec4 lightPos[4] = { cVec4(0.1f) ,cVec4(0.1f) ,cVec4(0.1f) ,cVec4(0.1f) }; // ���� ��ġ
        cFloat exposure = 0.0f;
        cFloat gamma = 0.0f;
        VkBool32 useTexture = VK_FALSE; // ���� Ȱ��ȭ ����
        VkBool32 brdfLUTTexture = VK_FALSE; // BRDF LUT �ؽ�ó ��� ����
        VkBool32 prefilteredCubeTexture = VK_FALSE; // Prefiltered Cube �ؽ�ó ��� ����
        VkBool32 irradianceCubeTexture = VK_FALSE; // Irradiance Cube �ؽ�ó ��� ����
    };

    struct subUinform : public VkBufferObject
    {
        subData subUniform = {};
    };

    class iblpbrEngine : public VulkanEngine {
    public:
        iblpbrEngine(std::string root_path);
        ~iblpbrEngine();
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

        Vk2DTexture* brdfLUTTexture = nullptr; // BRDF LUT �ؽ�ó
        VKcubeMap* prefilteredCubeTexture = nullptr; // Prefiltered Cube �ؽ�ó
        VKcubeMap* irradianceCubeTexture = nullptr; // Irradiance Cube �ؽ�ó

        subUinform subUniform; // ���� ������ ����

        // �Ʒ� �Լ����� ��ó
        // https://github.com/SaschaWillems/Vulkan/tree/master/examples/pbribl
        // ���� �����ϴ� ���� ���� ���� ���� �ɸ��� ������, ������ �������� ������ ��ü

        void generateBRDFLUT();
        void generatePrefilteredCube();
        void generateIrradianceCube();

        vkengine::helper::normalRander* normalRanderHelpe = nullptr; // Normal Rander ����
        cBool useNormalRander = true; // Normal Rander ��� ����
        cFloat normalRanderObjectScale = 0.1f; // Normal Rander ������Ʈ ������
#if 0
        object::ModelObject* vikingRoomObject = nullptr; // �� ������Ʈ2

        cUint selectModel = 0; // ���õ� �� �ε���
        std::vector<cString> modelNames = {}; // �� �̸���
        cString selectModelName = ""; // ���õ� �� �̸�

        cUint materialIndex = 0; // ��Ƽ���� �ε���
#endif

    };
}

#endif // !INCLUDE_IBLPBR_H_
