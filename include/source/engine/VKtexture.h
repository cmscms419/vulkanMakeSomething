#ifndef INCLUDE_VKTEXTURE_H_
#define INCLUDE_VKTEXTURE_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKdevice.h"
#include "VKengine.h"
#include "helper.h"

namespace vkengine {
    struct VkTextureBase {
    public:
        vkengine::VKdeviceHandler* device = NULL;             // Vulkan ����̽� ������
        VkImage image = VK_NULL_HANDLE;                 // �ؽ�ó �̹��� -> �ؽ�ó �̹����� �����ϴ� �� ���
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;    // �ؽ�ó �̹��� �޸� -> �ؽ�ó �̹����� �����ϴ� �� ���
        VkImageView imageView = VK_NULL_HANDLE;         // �ؽ�ó �̹��� �� -> �ؽ�ó �̹����� ��� ��ȯ (�̹��� ��� �̹����� �а� ���� �� ���)
        VkSampler sampler = VK_NULL_HANDLE;             // �ؽ�ó ���÷� -> �ؽ�ó �̹����� ���ø��ϴ� �� ���
        VkDescriptorImageInfo imageInfo{};              // �ؽ�ó �̹��� ���� -> �ؽ�ó �̹����� �����ϴ� �� ���
        TextureResourcePNG* resourcePNG = NULL;         // ���ҽ� ���� -> PNG �ؽ�ó ���ҽ��� �����ϴ� �� ���
        TextureResourceKTX* resourceKTX = NULL;         // KTX ���ҽ� ���� -> KTX �ؽ�ó ���ҽ��� �����ϴ� �� ���
        uint32_t VKmipLevels = 1;                       // Mipmap ���� -> �ؽ�ó�� Mipmap ������ �����ϴ� �� ���
        uint32_t imageCount = 0;                        // �̹��� ���� -> �ؽ�ó �̹����� ������ �����ϴ� �� ���

        virtual void createTextureImage() = 0; ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImageView(VkFormat format) = 0; ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler() = 0; ///< �ؽ�ó ���÷� ���� �Լ�
        virtual void createTextureImgae2(VkFormat format) { _PRINT_TO_CONSOLE_("CLEATE FUNTION"); };
        void setResourcePNG(TextureResourcePNG* resourcePNG); ///< ���ҽ� ���� �Լ�
        void setResourceKTX(TextureResourceKTX* resourceKTX) { this->resourceKTX = resourceKTX; } ///< KTX ���ҽ� ���� �Լ�
        void setDevice(vkengine::VKdeviceHandler* device) { this->device = device; } ///< ����̽� ���� �Լ�
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
        virtual void createTextureImgae2(VkFormat format) {};
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�

        Vk2DTexture() = default;
    };
#if 1
    struct Vk2DTextureArray : public VkTextureBase {

        virtual void createTextureImage(); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImgae2(VkFormat format) {};
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�

        Vk2DTextureArray() = default;
    };

    struct VKcubeMap : public VkTextureBase {
        virtual void createTextureImage(); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImgae2(VkFormat format);
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�

        VKcubeMap() = default;
    };
#endif
}
#endif // !INCLUDE_VKTEXTURE_H_
