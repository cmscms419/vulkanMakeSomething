#ifndef INCLUDE_VKTEXTURE_H_
#define INCLUDE_VKTEXTURE_H_

#include "../../common/common.h"
#include "../../common/struct.h"
#include "../../common/macros.h"

#include "VKdevice.h"

struct VkTexture_ {

    vkengine::VKDevice_* device = VK_NULL_HANDLE;                   // Vulkan ��ġ �ڵ�
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;   // Vulkan ���� ��ġ �ڵ�
    VkImage image = VK_NULL_HANDLE;                 // �ؽ�ó �̹��� -> �ؽ�ó �̹����� �����ϴ� �� ���
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;    // �ؽ�ó �̹��� �޸� -> �ؽ�ó �̹����� �����ϴ� �� ���
    VkImageView imageView = VK_NULL_HANDLE;         // �ؽ�ó �̹��� �� -> �ؽ�ó �̹����� ��� ��ȯ (�̹��� ��� �̹����� �а� ���� �� ���)
    VkSampler sampler = VK_NULL_HANDLE;             // �ؽ�ó ���÷� -> �ؽ�ó �̹����� ���ø��ϴ� �� ���
    uint32_t VKmipLevels;                               // �� ���� -> �̹����� ���� ����

    int texWidth = 0;                         ///< �ؽ�ó �ʺ�
    int texHeight = 0;                        ///< �ؽ�ó ����
    uint32_t texChannels = 0;                      ///< �ؽ�ó ä��
    std::string texPath = "";                      ///< �ؽ�ó ���

    //VkDevice device = VK_NULL_HANDLE; ///< Vulkan ��ġ �ڵ�
    //VkDescriptorImageInfo descriptor{}; ///< Vulkan ��ũ���� �̹��� ����
    //VkFormat format; ///< �̹��� ����
    //VkExtent3D extent{}; ///< �̹��� ũ��
    //uint32_t mipLevels; ///< �̹��� ���÷���
    //uint32_t layerCount; ///< �̹��� ���̾� ��
    //VkImageLayout layout; ///< �̹��� ���̾ƿ�
    //VkImageAspectFlags aspectFlags; ///< �̹��� �׼��� �÷���
    //VkImageUsageFlags usageFlags; ///< �̹��� ��� �÷���
    //VkMemoryPropertyFlags memoryPropertyFlags; ///< �޸� �Ӽ� �÷���
    //VkSamplerAddressMode addressMode; ///< ���÷� �ּ� ���
    //VkFilter filter; ///< ���÷� ����
    //VkSamplerMipmapMode mipmapMode; ///< ���÷� ���� ���
    //VkComponentMapping componentMapping{}; ///< ������Ʈ ����
    //VkImageSubresourceRange subresourceRange{}; ///< �̹��� ���긮�ҽ� ������
    //VkImageSubresourceLayers subresourceLayers{}; ///< �̹��� ���긮�ҽ� ���̾�
    //VkImageCreateInfo imageInfo{}; ///< �̹��� ���� ����
    //VkImageViewCreateInfo viewInfo{}; ///< �̹��� �� ���� ����
    //VkSamplerCreateInfo samplerInfo{}; ///< ���÷� ���� ����
    //VkDescriptorImageInfo descriptorInfo{}; ///< ��ũ���� �̹��� ����
    //VkImageSubresource subresource{}; ///< �̹��� ���긮�ҽ�

    void cleanup(); ///< �ؽ�ó ���� �Լ�

};

struct Vk2DTexture_ : public VkTexture_ {

    void createTextureImage(int type); ///< �ؽ�ó �̹��� ���� �Լ�
    void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
    void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�
};


#endif // !INCLUDE_VKTEXTURE_H_
