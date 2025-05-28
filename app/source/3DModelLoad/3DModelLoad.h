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
        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex) override; // 커맨드 버퍼 레코드
    private:

        // 각 3d 모델을 생성하기 위한 함수
        void createVertexbuffer();
        void createIndexBuffer();
        void createUniformBuffers();

        // Descriptor의 set, pool, layout을 생성하기 위한 함수들
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();

        void createDescriptorPool2_(uint32_t max_sets);
        void createDescriptorSets2();

        // grapics pipeline을 생성하기 위한 함수
        void createGraphicsPipeline();
        void createGraphicsPipeline2();

        void updateUniformBuffer(uint32_t currentImage);
        void updateUniformBuffer2(uint32_t currentImage, glm::mat4 matrix);
        
        void cleanupSwapcChain();

        // imgui 관련
        void initUI();
        vkGUI* vkGUI = nullptr;

        // 3d 모델을 위한 vertex, index buffer
        object::ModelObject modelObject; // viking_room

        // 3d object
        //object::TextureArrayObject3D cubeObject;

        // skybox
        object::SkyBox cubeSkybox;

        std::vector<subUniformBuffer> VKuniformBuffer = {};
        std::vector<subUniformBuffer> VKuniformBuffer2 = {};

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;                      // 그래픽스 파이프라인 -> 그래픽스 파이프라인을 생성
        VkPipeline VKCubeMapPipeline = VK_NULL_HANDLE;                       // 큐브맵 파이프라인 -> 큐브맵 파이프라인을 생성

        std::vector<VkDescriptorSet> VKdescriptorLoadModelSets = {};

        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };
    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
