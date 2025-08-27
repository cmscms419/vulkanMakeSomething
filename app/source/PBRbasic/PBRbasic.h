#ifndef INCLUDE_SKYMAP_IMGUI_H_
#define INCLUDE_SKYMAP_IMGUI_H_

#include "VKengine.h"
#include "VKtexture.h"

#include "Camera.h"
#include "object3D.h"
#include "VKimgui.h"
#include "VKengineOBJLoader.h"
#include "VKmodelDescriptor.h"
#include "VKSkymapModelDescriptor.h"

namespace vkengine
{
    struct subData
    {
        cVec4 camPos = cVec4(0.0f);
        cVec4 lightPos[4] = { cVec4(0.0f) ,cVec4(0.0f) ,cVec4(0.0f) ,cVec4(0.0f) }; // 조명 위치
        cVec4 objectPos = cVec4(0.0f);
        cBool useTexture = false; // 조명 활성화 여부
    };

    struct subUinform  : public VkBaseBuffer
    {
        subData subUniform = {};
    };

    class PBRbasuceEngine : public VulkanEngine {
    public:
        PBRbasuceEngine(std::string root_path);
        ~PBRbasuceEngine();
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
        void createDescriptor();

        void createGraphicsPipeline();
        void createGraphicsPipeline_skymap();
        void initUI();

        void cleanupSwapcChain();

        // imgui 관련
        vkGUI* vkGUI = nullptr;

        object::SkyBox* skyBox = nullptr; // 스카이박스
        object::ModelObject* modelObject = nullptr; // 모델 오브젝트
        object::ModelObject* vikingRoomObject = nullptr; // 모델 오브젝트2

        VkPipeline VKSkyMapPipeline = VK_NULL_HANDLE;       // 큐브맵 파이프라인 -> 큐브맵 파이프라인을 생성
        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;     // 모델 오브젝트 파이프라인

        VKDescriptor2* modeltDescriptor2 = nullptr;         // 모델 오브젝트 디스크립터
        VKDescriptor2* skyboxDescriptor2 = nullptr;         // 모델 오브젝트 디스크립터

        cUint selectModel = 0; // 선택된 모델 인덱스
        std::vector<cString> modelNames = { "Sphere", "Viking Room" }; // 모델 이름들
        cString selectModelName = "Sphere"; // 선택된 모델 이름

        cMaterial defaultMaterial; // 기본 머티리얼
        MaterialBuffer material; // 머티리얼 버퍼

        subUinform subUniform; // 서브 유니폼 버퍼
    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
