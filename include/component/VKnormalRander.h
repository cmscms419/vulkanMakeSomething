#ifndef _INCLUDE_VKNORMALRANDER_INCLUDE_
#define _INCLUDE_VKNORMALRANDER_INCLUDE_

#include "VKengine.h"

#include "VKDescriptor.h"
#include "Camera.h"
#include "object3D.h"

namespace vkengine
{
    namespace helper
    {
        class normalRander
        {
        public:
            // Normal Rander�� ���� ���� �Լ���
            normalRander(VulkanEngine* engine) : engine(engine) {};

            void cleanup();
            void createNormalObject(vkengine::object::Object3d* object);
            void createNormalRanderPipeline();
            void recordNormalRanderCommandBuffer(FrameData* frameData, uint32_t imageIndex);
            void cleanupNormalRanderPipeline();
            void updateCameraUniformBuffer(uint32_t currentImage, cMat4 world, vkengine::object::Camera* camera);
            void setNormalRanderObjectScale(cFloat scale) { this->normalRanderObjectScale = scale; }
        private:

            // Normal Rander ���������� ���� ������
            VulkanEngine* engine{ nullptr };
            VkPipeline normalRanderPipeline{};
            VKDescriptor2* normalRanderDescriptor{};

            UniformBuffer normalRanderUniformBuffer[2] = {}; // Normal Rander�� ���� Uniform Buffer
            vkengine::object::ModelObject* normalRanderObject{}; // Normal Rander�� ���� ������Ʈ
            cFloat normalRanderObjectScale = 1.0f; // Normal Rander ������Ʈ ������
        };
    }
}

#endif //_INCLUDE_VKNORMALRANDER_INCLUDE_