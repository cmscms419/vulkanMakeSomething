#ifndef  INCLUDE_GLTFLOADER_H_
#define  INCLUDE_GLTFLOADER_H_

#include "VKengine.h"

#include "object3D.h"
#include "VKimgui.h"
#include "VKengineGLTFLoader.h"

namespace vkengine
{


    class gltfLoaderEngine : public VulkanEngine {

    public:
        gltfLoaderEngine(std::string root_path);
        ~gltfLoaderEngine();
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

        struct ShaderData {
            VkBaseBuffer buffer{ sizeof(Values), "ShaderDataBuffer" };
            
            struct Values {
                cMat4 projection = cMat4(1.0f);
                cMat4 model = cMat4(1.0f);
                cVec4 lightPos = cVec4(5.0f, 5.0f, -5.0f, 1.0f); 
                cVec4 viewPos = cVec4(0.0f);
            } values;

        } shaderData;

        VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
        DescriptorManager manager; // 디스크립터 풀과 레이아웃을 관리하는 객체

        struct DescriptorSetlayoutAndSet {
            cString name = ""; // 디스크립터 세트 이름
            VkDescriptorSetLayout layout{ VK_NULL_HANDLE }; // 기본 레이아웃
            VkDescriptorSet descriptorSet{ VK_NULL_HANDLE }; // 기본 디스크립터 세트
        } matrixDescritor;

        // imgui 관련
        vkGUI* vkGUI = nullptr;
        vkengine::object::GLTFmodelObject* modelObject = nullptr;

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;     // 모델 오브젝트 파이프라인
        VkDescriptorPool VKModelDescriptorPool{ VK_NULL_HANDLE };

        // 각 3d 모델을 생성하기 위한 함수
        void createVertexbuffer();
        void createIndexBuffer();
        void createUniformBuffers();

        void createDescriptor();

        void createGraphicsPipeline();
        void initUI();

        void cleanupSwapcChain();

        // 임시적인 함수 -> 나중에 정리해서 어떻게 할지 정의해야 함
        void drawGLTF(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
        void drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Node* node, cMat4 modelObjectMatrix);

    };
}

#endif // ! INCLUDE_GLTFLOADER_H_
