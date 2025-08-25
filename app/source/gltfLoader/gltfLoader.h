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
        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex) override; // Ŀ�ǵ� ���� ���ڵ�

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
        DescriptorManager manager; // ��ũ���� Ǯ�� ���̾ƿ��� �����ϴ� ��ü

        struct DescriptorSetlayoutAndSet {
            cString name = ""; // ��ũ���� ��Ʈ �̸�
            VkDescriptorSetLayout layout{ VK_NULL_HANDLE }; // �⺻ ���̾ƿ�
            VkDescriptorSet descriptorSet{ VK_NULL_HANDLE }; // �⺻ ��ũ���� ��Ʈ
        } matrixDescritor;

        // imgui ����
        vkGUI* vkGUI = nullptr;
        vkengine::object::GLTFmodelObject* modelObject = nullptr;

        VkPipeline VKgraphicsPipeline = VK_NULL_HANDLE;     // �� ������Ʈ ����������
        VkDescriptorPool VKModelDescriptorPool{ VK_NULL_HANDLE };

        // �� 3d ���� �����ϱ� ���� �Լ�
        void createVertexbuffer();
        void createIndexBuffer();
        void createUniformBuffers();

        void createDescriptor();

        void createGraphicsPipeline();
        void initUI();

        void cleanupSwapcChain();

        // �ӽ����� �Լ� -> ���߿� �����ؼ� ��� ���� �����ؾ� ��
        void drawGLTF(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
        void drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Node* node, cMat4 modelObjectMatrix);

    };
}

#endif // ! INCLUDE_GLTFLOADER_H_
