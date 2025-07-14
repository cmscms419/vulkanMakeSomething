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
#include "VKGeometryGenerator.h"

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

    struct MaterialTexture
    {
        Vk2DTexture albedoMap;
        Vk2DTexture normalMap;
        Vk2DTexture aoMap;
        Vk2DTexture metallicMap;
        Vk2DTexture heightMap;
        Vk2DTexture roughnessMap;
    };

    struct subUinform : public VkBufferObject
    {
        subData subUniform = {};
    };

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
        MaterialTexture materialTexture; // 머티리얼 텍스처
        
        VkPipeline VKSkyMapPipeline = VK_NULL_HANDLE;       // 큐브맵 파이프라인 -> 큐브맵 파이프라인을 생성
        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;     // 모델 오브젝트 파이프라인

        UniformBufferSkymap uboParams; // 스카이박스 유니폼 버퍼 파라미터
        subUinform subUniform; // 서브 유니폼 버퍼
        
        VKDescriptor2* skyboxDescriptor2 = nullptr;         // 모델 오브젝트 디스크립터
        VKDescriptor2* modeltDescriptor2 = nullptr;         // 모델 오브젝트 디스크립터

        Vk2DTexture* brdfLUTTexture = nullptr; // BRDF LUT 텍스처
        VKcubeMap* prefilteredCubeTexture = nullptr; // Prefiltered Cube 텍스처
        VKcubeMap* irradianceCubeTexture = nullptr; // Irradiance Cube 텍스처

        void generateBRDFLUT();
        void generatePrefilteredCube();
        void generateIrradianceCube();
        
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
