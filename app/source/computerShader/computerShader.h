#ifndef INCLUDE_COMPUTERSHADER_H_
#define INCLUDE_COMPUTERSHADER_H_

#include <random>

#include "VKengine.h"
#include "VKtexture.h"

#include "Camera.h"
#include "object3D.h"
#include "VKimgui.h"
#include "VKloadModel.h"
#include "VKmodelDescriptor.h"
#include "VKSkymapModelDescriptor.h"
#include "VKGeometryGenerator.h"

#define MAX_PARTICALES 8192

namespace vkengine
{

    class computerShaderEngine : public VulkanEngine {
    public:
        computerShaderEngine(std::string root_path);
        ~computerShaderEngine();
        virtual void init() override;
        virtual bool prepare() override;
        virtual void cleanup() override;
        virtual void drawFrame() override;
        virtual bool mainLoop() override;
        void update(float dt);

    protected:
        virtual bool init_sync_structures() override;
        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex) override; // 커맨드 버퍼 레코드
        void recordComputerCommandBuffer(ComputerFrameData* framedata); // 컴퓨트 셰이더 커맨드 버퍼 레코드
    private:
        // computerCommandBuffer 생성
        void createComputerCommandBuffer();

        // 각 3d 모델을 생성하기 위한 함수
        void createVertexbuffer();
        void createIndexBuffer();
        void createUniformBuffers();
        void createShaderStorageBuffers();
        
        void createDescriptor();
        void initUI();

        void createComputerShaderPipeline();
        void createGraphicsPipeline();

        void cleanupSwapcChain();

        // imgui 관련
        vkGUI* vkGUI = nullptr;

        Particle particles[MAX_PARTICALES]; // 파티클 개수
        UniformBufferTime uboTime; // 시간 관련 uniform buffer
        
        VkBuffer shaderStorageBuffers[2];
        VkDeviceMemory shaderStorageBuffersMemory[2];

        VkBuffer uboTimeBuffers[2];
        VkDeviceMemory uboTimeBuffersMemory[2];
        
        void* uboTimemapped[2] = { nullptr, nullptr }; ///< 매핑된 메모리 포인터

        VkPipeline computershaderPipeline; // 컴퓨트 셰이더 파이프라인
        VkPipeline graphicsPipeline; // 그래픽스 파이프라인

        VkPipelineLayout pipelineLayout; // 파이프라인 레이아웃

        ComputerFrameData computeFrameData[MAX_FRAMES_IN_FLIGHT]; // 컴퓨트 셰이더 프레임 데이터

        VKDescriptor2* computershaderDrscriptor = nullptr;         // 모델 오브젝트 디스크립터

    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
