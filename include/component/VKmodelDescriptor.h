#ifndef INCLUDE_VKMODELDESCRIPTOR_H_
#define INCLUDE_VKMODELDESCRIPTOR_H_

#include "VKengine.h"
#include "VKdescriptor.h"
#include "VKtexture.h"

#include "object3D.h"


#define MAX_UNIFORM_BUFFER_COUNT 2

namespace vkengine {

    // 이것은 안사용할 예정
    // 활용하기가 어렵다.
    struct VK3DModelDescriptor : public VKDescriptor
    {
    public:
        VK3DModelDescriptor(VkDevice device = VK_NULL_HANDLE, cUint16_t frames = 0) : VKDescriptor(device, frames) {};

        virtual void createDescriptorSetLayout(bool useTexture) override;

        // <summary>
        // descriptorCount는 디스크립터 풀의 개수를 의미합니다. 풀에 몇 개의 uniform buffer 디스크립터를 만들 수 있는지를 지정하는 값입니다.
        // </summary>
        virtual void createDescriptorPool(bool useTexture) override;

        virtual void createDescriptorSets(bool useTexture) override;

        virtual void updateDescriptorSets() override;

        virtual void BindDescriptorSets(VkCommandBuffer mainCommandBuffer, size_t currentFrame, cUint16_t offset) override;

        void createPipelineLayout();

        void setObject(object::Object3d* object) { this->objects.push_back(object); }

        virtual cUint16_t getDescriptorCount() { return static_cast<cUint16_t>(this->objects.size()); } // 디스크립터 개수 반환
        virtual VkPipelineLayout getPipelineLayout() { return this->VKpipelineLayout; } // 파이프라인 레이아웃 반환

    private:
        // 0: Uniform buffer (Vertex shader)
        // 1: Texture sampler (texture Fragment shader)

        VkDescriptorPoolSize poolSizes[MAX_UNIFORM_BUFFER_COUNT]{}; // 디스크립터 풀 사이즈(임시적)
        VkDescriptorSetLayoutBinding uboLayoutBinding[MAX_UNIFORM_BUFFER_COUNT]{}; // 디스크립터 세트 레이아웃 바인딩

        std::vector<object::Object3d*> objects{}; // 3D 모델 객체들
    };

} // namespace vkengine

#endif// INCLUDE_VKMODELDESCRIPTOR_H_