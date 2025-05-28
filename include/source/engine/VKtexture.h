#ifndef INCLUDE_VKTEXTURE_H_
#define INCLUDE_VKTEXTURE_H_

#include "../../common/common.h"
#include "../../common/struct.h"
#include "../../common/macros.h"

#include "VKdevice.h"
namespace vkengine {
    struct VkTextureBase {

        vkengine::VKDevice_* device = VK_NULL_HANDLE;   // Vulkan ��ġ �ڵ�

        VkImage image = VK_NULL_HANDLE;                 // �ؽ�ó �̹��� -> �ؽ�ó �̹����� �����ϴ� �� ���
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;    // �ؽ�ó �̹��� �޸� -> �ؽ�ó �̹����� �����ϴ� �� ���
        VkImageView imageView = VK_NULL_HANDLE;         // �ؽ�ó �̹��� �� -> �ؽ�ó �̹����� ��� ��ȯ (�̹��� ��� �̹����� �а� ���� �� ���)
        VkSampler sampler = VK_NULL_HANDLE;             // �ؽ�ó ���÷� -> �ؽ�ó �̹����� ���ø��ϴ� �� ���
        VkDescriptorImageInfo imageInfo{};              // �ؽ�ó �̹��� ���� -> �ؽ�ó �̹����� �����ϴ� �� ���

        uint32_t texWidth = 0;                         // �ؽ�ó �ʺ�
        uint32_t texHeight = 0;                        // �ؽ�ó ����
        uint32_t texChannels = 0;                      // �ؽ�ó ä��
        uint32_t VKmipLevels = 0;                      // �� ���� -> �̹����� �Ӹ� ����

        virtual void createTextureImage(int type, const char* texPath) = 0; ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImageView(VkFormat format) = 0; ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler() = 0; ///< �ؽ�ó ���÷� ���� �Լ�
        
        virtual void createDescriptorImageInfo()
        {
            this->imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            this->imageInfo.imageView = this->imageView;
            this->imageInfo.sampler = this->sampler;
        }; ///< �ؽ�ó �̹��� ���� ���� �Լ�

        inline const void cleanup() const  //< �ؽ�ó ���� �Լ�
        {
            vkDestroySampler(device->logicaldevice, sampler, nullptr);
            vkDestroyImageView(device->logicaldevice, imageView, nullptr);
            vkDestroyImage(device->logicaldevice, image, nullptr);
            vkFreeMemory(device->logicaldevice, imageMemory, nullptr);
        }

    };

    struct Vk2DTexture : public VkTextureBase {

        virtual void createTextureImage(int type, const char* texPath); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�
    };

    struct Vk2DTextureArray : public VkTextureBase {

        uint32_t imageCount = 0; ///< �̹��� ����

        void createTextureArrayImages(int type, std::vector<std::string>& texPath); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�
        virtual void createTextureImage(int type, const char* texPath) {}; ///< �ؽ�ó �̹��� ���� �Լ�
    };

    struct VKcubeMap : public VkTextureBase {
        void createTextureCubeImages(int type, std::vector<std::string>& texPath); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�
        virtual void createTextureImage(int type, const char* texPath) {}; ///< �ؽ�ó �̹��� ���� �Լ�
    };
}
#endif // !INCLUDE_VKTEXTURE_H_
