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
        virtual void recordCommandBuffer(FrameData* framedata, uint32_t imageIndex) override; // Ŀ�ǵ� ���� ���ڵ�
        void recordComputerCommandBuffer(ComputerFrameData* framedata); // ��ǻƮ ���̴� Ŀ�ǵ� ���� ���ڵ�
    private:
        // computerCommandBuffer ����
        void createComputerCommandBuffer();

        // �� 3d ���� �����ϱ� ���� �Լ�
        void createVertexbuffer();
        void createIndexBuffer();
        void createUniformBuffers();
        void createShaderStorageBuffers();
        
        void createDescriptor();
        void initUI();

        void createComputerShaderPipeline();
        void createGraphicsPipeline();

        void cleanupSwapcChain();

        // imgui ����
        vkGUI* vkGUI = nullptr;

        Particle particles[MAX_PARTICALES]; // ��ƼŬ ����
        UniformBufferTime uboTime; // �ð� ���� uniform buffer
        
        VkBuffer shaderStorageBuffers[2];
        VkDeviceMemory shaderStorageBuffersMemory[2];

        VkBuffer uboTimeBuffers[2];
        VkDeviceMemory uboTimeBuffersMemory[2];
        
        void* uboTimemapped[2] = { nullptr, nullptr }; ///< ���ε� �޸� ������

        VkPipeline computershaderPipeline; // ��ǻƮ ���̴� ����������
        VkPipeline graphicsPipeline; // �׷��Ƚ� ����������

        VkPipelineLayout pipelineLayout; // ���������� ���̾ƿ�

        ComputerFrameData computeFrameData[MAX_FRAMES_IN_FLIGHT]; // ��ǻƮ ���̴� ������ ������

        VKDescriptor2* computershaderDrscriptor = nullptr;         // �� ������Ʈ ��ũ����

    };
}

#endif // !INCLUDE_SKYMAP_IMGUI_H_
