#ifndef INCLUDE_VKSKYMAPMODELDESCRIPTOR_H_
#define INCLUDE_VKSKYMAPMODELDESCRIPTOR_H_

#include "VKengine.h"
#include "VKdescriptor.h"
#include "VKtexture.h"

#include "object3D.h"

#define MAX_UNIFORM_BUFFER_COUNT 3

namespace vkengine {
struct VKSkyMapModelDescriptor : public VKDescriptor {
 public:
  VKSkyMapModelDescriptor(VkDevice device = VK_NULL_HANDLE)
      : VKDescriptor(device) {};
  virtual void createDescriptorSetLayout(bool useTexture) override;
  /// <summary>
  /// descriptorCount�� ��ũ���� Ǯ�� ������ �ǹ��մϴ�. Ǯ�� �� ���� uniform
  /// buffer ��ũ���͸� ���� �� �ִ����� �����ϴ� ���Դϴ�.
  /// </summary>
  virtual void createDescriptorPool(bool useTexture) override;

  virtual void createDescriptorSets(bool useTexture) override;

  virtual void updateDescriptorSets() override;

  virtual void BindDescriptorSets(VkCommandBuffer mainCommandBuffer,
                                  size_t currentFrame,
                                  uint16_t offset) override;

  void createPipelineLayout();

  void setObject(object::Object* object) { this->objects.push_back(object); }

  uint16_t getDescriptorCount() {
    return this->objects.size();
  }  // ��ũ���� ���� ��ȯ
  VkPipelineLayout getPipelineLayout() {
    return this->VKpipelineLayout;
  }  // ���������� ���̾ƿ� ��ȯ

  private:
  // 0: Uniform buffer (Vertex shader)
  // 1: Texture sampler (cubmap Fragment shader)
  // 2: Texture sampler (textureArray Fragment shader)

  VkDescriptorPoolSize poolSizes[MAX_UNIFORM_BUFFER_COUNT]{};  // ��ũ���� Ǯ ������(�ӽ���)
  VkDescriptorSetLayoutBinding uboLayoutBinding[MAX_UNIFORM_BUFFER_COUNT]{};  // ��ũ���� ��Ʈ ���̾ƿ� ���ε�

  std::vector<object::Object*> objects{};  // 3D �� ��ü��
  bool useTexture = false;  // �ؽ�ó ��� ����

};
}  // namespace vkengine

#endif  // !INCLUDE_VKSKYMAPMODELDESCRIPTOR_H_
