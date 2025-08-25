#ifndef INCLUDE_VKMODELDESCRIPTOR_H_
#define INCLUDE_VKMODELDESCRIPTOR_H_

#include "VKengine.h"
#include "VKdescriptor.h"
#include "VKtexture.h"

#include "object3D.h"


#define MAX_UNIFORM_BUFFER_COUNT 2

namespace vkengine {

    // �̰��� �Ȼ���� ����
    // Ȱ���ϱⰡ ��ƴ�.
    struct VK3DModelDescriptor : public VKDescriptor
    {
    public:
        VK3DModelDescriptor(VkDevice device = VK_NULL_HANDLE, cUint16_t frames = 0) : VKDescriptor(device, frames) {};

        virtual void createDescriptorSetLayout(bool useTexture) override;

        // <summary>
        // descriptorCount�� ��ũ���� Ǯ�� ������ �ǹ��մϴ�. Ǯ�� �� ���� uniform buffer ��ũ���͸� ���� �� �ִ����� �����ϴ� ���Դϴ�.
        // </summary>
        virtual void createDescriptorPool(bool useTexture) override;

        virtual void createDescriptorSets(bool useTexture) override;

        virtual void updateDescriptorSets() override;

        virtual void BindDescriptorSets(VkCommandBuffer mainCommandBuffer, size_t currentFrame, cUint16_t offset) override;

        void createPipelineLayout();

        void setObject(object::Object3d* object) { this->objects.push_back(object); }

        virtual cUint16_t getDescriptorCount() { return static_cast<cUint16_t>(this->objects.size()); } // ��ũ���� ���� ��ȯ
        virtual VkPipelineLayout getPipelineLayout() { return this->VKpipelineLayout; } // ���������� ���̾ƿ� ��ȯ

    private:
        // 0: Uniform buffer (Vertex shader)
        // 1: Texture sampler (texture Fragment shader)

        VkDescriptorPoolSize poolSizes[MAX_UNIFORM_BUFFER_COUNT]{}; // ��ũ���� Ǯ ������(�ӽ���)
        VkDescriptorSetLayoutBinding uboLayoutBinding[MAX_UNIFORM_BUFFER_COUNT]{}; // ��ũ���� ��Ʈ ���̾ƿ� ���ε�

        std::vector<object::Object3d*> objects{}; // 3D �� ��ü��
    };

} // namespace vkengine

#endif// INCLUDE_VKMODELDESCRIPTOR_H_