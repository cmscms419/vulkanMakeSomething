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
        cVec4 lightPos[4] = { cVec4(0.1f) ,cVec4(0.1f) ,cVec4(0.1f) ,cVec4(0.1f) }; // 조명 위치
        cFloat exposure = 0.0f;
        cFloat gamma = 0.0f;
        VkBool32 useTexture = VK_FALSE; // 조명 활성화 여부
        VkBool32 brdfLUTTexture = VK_FALSE; // BRDF LUT 텍스처 사용 여부
        VkBool32 prefilteredCubeTexture = VK_FALSE; // Prefiltered Cube 텍스처 사용 여부
        VkBool32 irradianceCubeTexture = VK_FALSE; // Irradiance Cube 텍스처 사용 여부
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

        Vk2DTexture* brdfLUTTexture = nullptr; // BRDF LUT 텍스처
        VKcubeMap* prefilteredCubeTexture = nullptr; // Prefiltered Cube 텍스처
        VKcubeMap* irradianceCubeTexture = nullptr; // Irradiance Cube 텍스처

        subUinform subUniform; // 서브 유니폼 버퍼

        // 아래 함수들의 출처
        // https://github.com/SaschaWillems/Vulkan/tree/master/examples/pbribl
        // 직접 구현하는 것을 보는 것이 오래 걸리기 때문에, 파일을 가져오는 것으로 대체

        void generateBRDFLUT();
        void generatePrefilteredCube();
        void generateIrradianceCube();

        vkengine::helper::normalRander* normalRanderHelpe = nullptr; // Normal Rander 헬퍼
        cBool useNormalRander = true; // Normal Rander 사용 여부
        cFloat normalRanderObjectScale = 0.1f; // Normal Rander 오브젝트 스케일
#if 0
        object::ModelObject* vikingRoomObject = nullptr; // 모델 오브젝트2

        cUint selectModel = 0; // 선택된 모델 인덱스
        std::vector<cString> modelNames = {}; // 모델 이름들
        cString selectModelName = ""; // 선택된 모델 이름

        cUint materialIndex = 0; // 머티리얼 인덱스
#endif

    };
}

#endif // !INCLUDE_IBLPBR_H_
