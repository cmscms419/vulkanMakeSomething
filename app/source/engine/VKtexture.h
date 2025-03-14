#ifndef INCLUDE_VKTEXTURE_H_
#define INCLUDE_VKTEXTURE_H_

#include "../../common/common.h"
#include "../../common/struct.h"
#include "../../common/macros.h"

#include "VKdevice.h"

struct VkTexture_ {

    vkengine::VKDevice_* device = VK_NULL_HANDLE;   // Vulkan ��ġ �ڵ�
    
    VkImage image = VK_NULL_HANDLE;                 // �ؽ�ó �̹��� -> �ؽ�ó �̹����� �����ϴ� �� ���
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;    // �ؽ�ó �̹��� �޸� -> �ؽ�ó �̹����� �����ϴ� �� ���
    VkImageView imageView = VK_NULL_HANDLE;         // �ؽ�ó �̹��� �� -> �ؽ�ó �̹����� ��� ��ȯ (�̹��� ��� �̹����� �а� ���� �� ���)
    VkSampler sampler = VK_NULL_HANDLE;             // �ؽ�ó ���÷� -> �ؽ�ó �̹����� ���ø��ϴ� �� ���
    
    uint32_t texWidth = 0;                         ///< �ؽ�ó �ʺ�
    uint32_t texHeight = 0;                        ///< �ؽ�ó ����
    uint32_t texChannels = 0;                      ///< �ؽ�ó ä��
    uint32_t VKmipLevels = 0;                      // �� ���� -> �̹����� �Ӹ� ����

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

    virtual void createTextureImage(int type, const char* texPath) = 0; ///< �ؽ�ó �̹��� ���� �Լ�
    virtual void createTextureImageView(VkFormat format) = 0; ///< �ؽ�ó �̹��� �� ���� �Լ�
    virtual void createTextureSampler() = 0; ///< �ؽ�ó ���÷� ���� �Լ�

    inline const void cleanup() const  //< �ؽ�ó ���� �Լ�
    {
        vkDestroySampler(device->logicaldevice, sampler, nullptr);
        vkDestroyImageView(device->logicaldevice, imageView, nullptr);
        vkDestroyImage(device->logicaldevice, image, nullptr);
        vkFreeMemory(device->logicaldevice, imageMemory, nullptr);
    }

};

struct Vk2DTexture : public VkTexture_ {

    virtual void createTextureImage(int type, const char* texPath); ///< �ؽ�ó �̹��� ���� �Լ�
    virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
    virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�
};

struct Vk2DTextureArray : public VkTexture_ {

    uint32_t imageCount = 0; ///< �̹��� ����

    void createTextureImages(int type, std::vector<std::string> &texPath); ///< �ؽ�ó �̹��� ���� �Լ�

    virtual void createTextureImage(int type, const char* texPath); ///< �ؽ�ó �̹��� ���� �Լ�
    virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
    virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�
};


#endif // !INCLUDE_VKTEXTURE_H_
