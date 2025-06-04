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
        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex) override; // 커맨드 버퍼 레코드

    private:

        // 각 3d 모델을 생성하기 위한 함수
        void createVertexbuffer();
        void createIndexBuffer();
        void createUniformBuffers();

        void createDescriptor();

        // grapics pipeline을 생성하기 위한 함수
        void createGraphicsPipeline();
        void createGraphicsPipeline2();
        void createGraphicsPipeline_skymap();

        void cleanupSwapcChain();

        // imgui 관련
        void initUI();
        vkGUI* vkGUI = nullptr;

        VK3DModelDescriptor* modelObjectDescriptor = nullptr;
        VKSkyMapModelDescriptor* skyMapModelDescriptor = nullptr;

        // 3d 모델을 위한 vertex, index buffer
        object::ModelObject* modelObject;  // viking_room
        object::ModelObject* modelObject2; // viking_room

        glm::mat4 worldMatrix = glm::mat4(1.0f); // 월드 매트릭스

        // skybox
        object::SkyBox* cubeSkybox;

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;                      // 그래픽스 파이프라인 -> 그래픽스 파이프라인을 생성
        VkPipeline VKgraphicsPipeline2 = VK_NULL_HANDLE;                     // 그래픽스 파이프라인2 -> cube 파이프라인을 생성
        VkPipeline VKSkyMapPipeline = VK_NULL_HANDLE;                       // 큐브맵 파이프라인 -> 큐브맵 파이프라인을 생성

        std::vector<VkDescriptorSet> VKdescriptorLoadModelSets = {};

        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE };
    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
