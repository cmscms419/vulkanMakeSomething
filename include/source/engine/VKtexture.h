#ifndef INCLUDE_VKTEXTURE_H_
#define INCLUDE_VKTEXTURE_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKdevice.h"
#include "VKengine.h"
#include "helper.h"

namespace vkengine {

    struct VKimageData {
        VkImage image = VK_NULL_HANDLE;                 // �ؽ�ó �̹��� -> �ؽ�ó �̹����� �����ϴ� �� ���
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;    // �ؽ�ó �̹��� �޸� -> �ؽ�ó �̹����� �����ϴ� �� ���
        VkImageView imageView = VK_NULL_HANDLE;         // �ؽ�ó �̹��� �� -> �ؽ�ó �̹����� ��� ��ȯ (�̹��� ��� �̹����� �а� ���� �� ���)
        VkSampler sampler = VK_NULL_HANDLE;             // �ؽ�ó ���÷� -> �ؽ�ó �̹����� ���ø��ϴ� �� ���
        VkDescriptorImageInfo imageInfo{};              // �ؽ�ó �̹��� ���� -> �ؽ�ó �̹����� �����ϴ� �� ���
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE; // ��ũ���� ��Ʈ -> �ؽ�ó �̹����� ����ϴ� ���̴��� ���޵Ǵ� ����

        void createDescriptorImageInfo()
        {
            this->imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            this->imageInfo.imageView = this->imageView;
            this->imageInfo.sampler = this->sampler;
        };
    };

    struct imageResource {
        TextureResourcePNG* resourcePNG{};     // ���ҽ� ���� -> PNG �ؽ�ó ���ҽ��� �����ϴ� �� ���
        TextureResourceKTX* resourceKTX{};     // KTX ���ҽ� ���� -> KTX �ؽ�ó ���ҽ��� �����ϴ� �� ���
    };

    struct VkTextureBase {
    public:
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };  // ���� ����̽� -> GPU Physical Handle
        VkDevice logicaldevice{ VK_NULL_HANDLE };           // �� ����̽� -> GPU Logical Handle
        VkCommandPool commandPool{ VK_NULL_HANDLE };        // Ŀ�ǵ� Ǯ -> Ŀ�ǵ� ���۸� �����ϴ� �� ���
        VkQueue graphicsVKQueue{ VK_NULL_HANDLE };          // �׷��Ƚ� ť -> �׷��Ƚ� ����� ó���ϴ� ť
        std::vector<VKimageData> imageData{};               // �ؽ�ó �̹��� ������ -> GPU�� ���� ������
        std::vector<imageResource> imageTextureDatas{};     // �ؽ�ó �̹��� ������ -> ���ҽ� ������
        cUint32_t VKmipLevels = 1;                          // �ؽ�ó�� Mip ���� ���� ����

        virtual void setTexturePNG(TextureResourcePNG* resourcePNG)
        {
            if (resourcePNG == nullptr) {
                _PRINT_TO_CONSOLE_("TextureResourcePNG is nullptr");
                return;
            }

            imageResource imageData{};
            imageData.resourcePNG = resourcePNG;
            this->imageTextureDatas.push_back(imageData);
        }
        virtual void setTextureKTX(TextureResourceKTX* resourceKTX)
        {
            if (resourceKTX == nullptr) {
                _PRINT_TO_CONSOLE_("TextureResourceKTX is nullptr");
                return;
            }

            imageResource imageData{};
            imageData.resourceKTX = resourceKTX;
            this->imageTextureDatas.push_back(imageData);
        }

        virtual void createTextureImagePNG() = 0; ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImgaeKTX(VkFormat format) { _PRINT_TO_CONSOLE_("CLEATE FUNTION"); };
        virtual void createTextureImageView(VkFormat format) = 0; ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler() = 0; ///< �ؽ�ó ���÷� ���� �Լ�
        virtual void cleanResource()
        {
            for (auto& imageTextureData : this->imageTextureDatas)
            {
                if (imageTextureData.resourcePNG)
                {
                    delete imageTextureData.resourcePNG; // ���ҽ� ����
                }
                if (imageTextureData.resourceKTX)
                {
                    delete imageTextureData.resourceKTX; // ���ҽ� ����
                }
            }
            this->imageTextureDatas.clear();
        }

        virtual void cleanup() //< �ؽ�ó ���� �Լ�
        {
            for (VKimageData& data : this->imageData)
            {
                vkDestroySampler(this->logicaldevice, data.sampler, nullptr);
                vkDestroyImageView(this->logicaldevice, data.imageView, nullptr);
                vkDestroyImage(this->logicaldevice, data.image, nullptr);
                vkFreeMemory(this->logicaldevice, data.imageMemory, nullptr);
            }
        }

        virtual void createDescriptorImageInfo() ///< �ؽ�ó �̹��� ���� ���� �Լ�
        {
            for (VKimageData& data : this->imageData) {
                data.createDescriptorImageInfo();
            }
        }
        
        void initializeDeviceHandles(VkPhysicalDevice physicalDevice, VkDevice logicaldevice, VkCommandPool commandPool, VkQueue graphicsVKQueue) ///< ����̽� ���� �Լ�
        {
            this->physicalDevice = physicalDevice;
            this->logicaldevice = logicaldevice;
            this->commandPool = commandPool;
            this->graphicsVKQueue = graphicsVKQueue;
        }

        VkTextureBase() = default; ///< �⺻ ������

    };

    struct Vk2DTexture : public VkTextureBase {
        virtual void createTextureImagePNG(); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImgaeKTX(VkFormat format) {};
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�

        Vk2DTexture() = default;
    };

    struct Vk2DArrayTexture : public VkTextureBase {

        // https://github.com/SaschaWillems/Vulkan �� ����Ʈ���� texturearray ����
        // �ش� ������ ���ҽ��� �迭 ���·� �Ǿ� �־, �迭 ���·� �ؽ�ó�� �ε��ϴ� �����̴�.
        // �� ������ ���� ���ҽ� �������� �������� ���·� ������ ���ҽ��� �����´�.
        virtual void createTextureImagePNG(); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImgaeKTX(VkFormat format) {};
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�

        Vk2DArrayTexture() = default;
    private:
        cUint32_t imageCount = 0;   // �̹��� ���� -> �ؽ�ó �̹����� ������ ����
    };

    struct VKcubeMap : public VkTextureBase {
        virtual void createTextureImagePNG(); ///< �ؽ�ó �̹��� ���� �Լ�
        virtual void createTextureImgaeKTX(VkFormat format);
        virtual void createTextureImageView(VkFormat format); ///< �ؽ�ó �̹��� �� ���� �Լ�
        virtual void createTextureSampler(); ///< �ؽ�ó ���÷� ���� �Լ�
    
        VKcubeMap() = default;
    };
}
#endif // !INCLUDE_VKTEXTURE_H_
