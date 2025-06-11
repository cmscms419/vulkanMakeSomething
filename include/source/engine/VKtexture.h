#ifndef INCLUDE_VKTEXTURE_H_
#define INCLUDE_VKTEXTURE_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKdevice.h"
#include "VKengine.h"

namespace vkengine {
    struct VkTextureBase {
    protected:
        vkengine::VKDevice_* device = NULL; ///< Vulkan ����̽� ������
        VkImage image = VK_NULL_HANDLE;                 // �ؽ�ó �̹��� -> �ؽ�ó �̹����� �����ϴ� �� ���
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;    // �ؽ�ó �̹��� �޸� -> �ؽ�ó �̹����� �����ϴ� �� ���
        VkImageView imageView = VK_NULL_HANDLE;         // �ؽ�ó �̹��� �� -> �ؽ�ó �̹����� ��� ��ȯ (�̹��� ��� �̹����� �а� ���� �� ���)
        VkSampler sampler = VK_NULL_HANDLE;             // �ؽ�ó ���÷� -> �ؽ�ó �̹����� ���ø��ϴ� �� ���
        VkDescriptorImageInfo imageInfo{};              // �ؽ�ó �̹��� ���� -> �ؽ�ó �̹����� �����ϴ� �� ���
        TextureResource* resource = NULL;               // ���ҽ� ���� -> �ؽ�ó ���ҽ��� �����ϴ� �� ���
        uint32_t VKmipLevels = 1;                       // Mipmap ���� -> �ؽ�ó�� Mipmap ������ �����ϴ� �� ���
        uint32_t imageCount = 0;                          // �̹��� ���� -> �ؽ�ó �̹����� ������ �����ϴ� �� ���

    public:

        VkImage getImage() const { return image; } ///< �ؽ�ó �̹��� ��ȯ �Լ�
        VkDeviceMemory getImageMemory() const { return imageMemory; } ///< �ؽ�ó �̹��� �޸� ��ȯ �Լ�
        VkImageView getImageView() const { return imageView; } ///< �ؽ�ó �̹��� �� ��ȯ �Լ�
        VkSampler getSampler() const { return sampler; } ///< �ؽ�ó ���÷� ��ȯ �Լ�
        VkDescriptorImageInfo getImageInfo() const { return imageInfo; } ///< �ؽ�ó �̹��� ���� ��ȯ �Լ�
        TextureResource* getResource() const { return resource; } ///< ���ҽ� ���� ��ȯ �Լ�
        uint32_t getMipLevels() const { return VKmipLevels; } ///< Mipmap ���� ��ȯ �Լ�
        uint32_t getImageCount() const { return imageCount; } ///< �̹��� ���� ��ȯ �Լ�

        virtual void createTextureImage() = 0; ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImageView(VkFormat format) = 0; ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler() = 0; ///< �ؽ�ó ���÷� ���� �Լ�
        void setResource(TextureResource* resource); ///< ���ҽ� ���� �Լ�
        void setDevice(vkengine::VKDevice_* device) { this->device = device; } ///< ����̽� ���� �Լ�
        void setMipLevels(uint32_t mipLevels) { this->VKmipLevels = mipLevels; } ///< Mipmap ���� ���� �Լ�

        ///< �ؽ�ó �̹��� ���� ���� �Լ�
        void createDescriptorImageInfo()
        {
            this->imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            this->imageInfo.imageView = this->imageView;
            this->imageInfo.sampler = this->sampler;
        }; 

        const void cleanup()  //< �ؽ�ó ���� �Լ�
        {
            vkDestroySampler(device->logicaldevice, sampler, nullptr);
            vkDestroyImageView(device->logicaldevice, imageView, nullptr);
            vkDestroyImage(device->logicaldevice, image, nullptr);
            vkFreeMemory(device->logicaldevice, imageMemory, nullptr);
        }
    };

    struct Vk2DTexture : public VkTextureBase {
        virtual void createTextureImage(); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�

        Vk2DTexture() = default;
    };
#if 1
    struct Vk2DTextureArray : public VkTextureBase {

        virtual void createTextureImage(); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�

        Vk2DTextureArray() = default;
    };

    struct VKcubeMap : public VkTextureBase {
        virtual void createTextureImage(); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�

        VKcubeMap() = default;
    };
#endif
}
#endif // !INCLUDE_VKTEXTURE_H_
