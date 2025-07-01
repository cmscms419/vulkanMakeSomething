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

        // skymap 관련
        object::SkyBox* skyBox = nullptr; // 스카이박스 오브젝트
        object::ModelObject* modelObject = nullptr; // 모델 오브젝트

        MaterialBuffer material; // 머티리얼 버퍼
        
        VkPipeline VKSkyMapPipeline = VK_NULL_HANDLE;       // 큐브맵 파이프라인 -> 큐브맵 파이프라인을 생성
        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;     // 모델 오브젝트 파이프라인

        UniformBufferSkymap uboParams; // 스카이박스 유니폼 버퍼 파라미터
        
        VKDescriptor2* skyboxDescriptor2 = nullptr;         // 모델 오브젝트 디스크립터
        VKDescriptor2* modeltDescriptor2 = nullptr;         // 모델 오브젝트 디스크립터
        
#if 0
        object::ModelObject* vikingRoomObject = nullptr; // 모델 오브젝트2


        cUint selectModel = 0; // 선택된 모델 인덱스
        std::vector<cString> modelNames = {}; // 모델 이름들
        cString selectModelName = ""; // 선택된 모델 이름

        cUint materialIndex = 0; // 머티리얼 인덱스
#endif

    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
