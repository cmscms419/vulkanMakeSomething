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
        vkengine::VKdeviceHandler* device = NULL;             // Vulkan 디바이스 포인터
        VkImage image = VK_NULL_HANDLE;                 // 텍스처 이미지 -> 텍스처 이미지를 저장하는 데 사용
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;    // 텍스처 이미지 메모리 -> 텍스처 이미지를 저장하는 데 사용
        VkImageView imageView = VK_NULL_HANDLE;         // 텍스처 이미지 뷰 -> 텍스처 이미지를 뷰로 변환 (이미지 뷰는 이미지를 읽고 쓰는 데 사용)
        VkSampler sampler = VK_NULL_HANDLE;             // 텍스처 샘플러 -> 텍스처 이미지를 샘플링하는 데 사용
        VkDescriptorImageInfo imageInfo{};              // 텍스처 이미지 정보 -> 텍스처 이미지를 설명하는 데 사용
        TextureResourcePNG* resourcePNG = NULL;         // 리소스 정보 -> PNG 텍스처 리소스를 설명하는 데 사용
        TextureResourceKTX* resourceKTX = NULL;         // KTX 리소스 정보 -> KTX 텍스처 리소스를 설명하는 데 사용
        uint32_t VKmipLevels = 1;                       // Mipmap 레벨 -> 텍스처의 Mipmap 레벨을 저장하는 데 사용
        uint32_t imageCount = 0;                        // 이미지 개수 -> 텍스처 이미지의 개수를 저장하는 데 사용

        virtual void createTextureImage() = 0; ///< 텍스처 이미지 생성 함수
        virtual void createTextureImageView(VkFormat format) = 0; ///< 텍스처 이미지 뷰 생성 함수
        virtual void createTextureSampler() = 0; ///< 텍스처 샘플러 생성 함수
        virtual void createTextureImgae2(VkFormat format) { _PRINT_TO_CONSOLE_("CLEATE FUNTION"); };
        void setResourcePNG(TextureResourcePNG* resourcePNG); ///< 리소스 설정 함수
        void setResourceKTX(TextureResourceKTX* resourceKTX) { this->resourceKTX = resourceKTX; } ///< KTX 리소스 설정 함수
        void setDevice(vkengine::VKdeviceHandler* device) { this->device = device; } ///< 디바이스 설정 함수
        void setMipLevels(uint32_t mipLevels) { this->VKmipLevels = mipLevels; } ///< Mipmap 레벨 설정 함수

        ///< 텍스처 이미지 정보 생성 함수
        void createDescriptorImageInfo()
        {
            this->imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            this->imageInfo.imageView = this->imageView;
            this->imageInfo.sampler = this->sampler;
        }; 

        const void cleanup()  //< 텍스처 정리 함수
        {
            vkDestroySampler(device->logicaldevice, sampler, nullptr);
            vkDestroyImageView(device->logicaldevice, imageView, nullptr);
            vkDestroyImage(device->logicaldevice, image, nullptr);
            vkFreeMemory(device->logicaldevice, imageMemory, nullptr);
        }
    };

    struct Vk2DTexture : public VkTextureBase {
        virtual void createTextureImage(); ///< 텍스처 이미지 생성 함수
        virtual void createTextureImgae2(VkFormat format) {};
        virtual void createTextureImageView(VkFormat format); ///< 텍스처 이미지 뷰 생성 함수
        virtual void createTextureSampler(); ///< 텍스처 샘플러 생성 함수

        Vk2DTexture() = default;
    };
#if 1
    struct Vk2DTextureArray : public VkTextureBase {

        virtual void createTextureImage(); ///< 텍스처 이미지 생성 함수
        virtual void createTextureImgae2(VkFormat format) {};
        virtual void createTextureImageView(VkFormat format); ///< 텍스처 이미지 뷰 생성 함수
        virtual void createTextureSampler(); ///< 텍스처 샘플러 생성 함수

        Vk2DTextureArray() = default;
    };

    struct VKcubeMap : public VkTextureBase {
        virtual void createTextureImage(); ///< 텍스처 이미지 생성 함수
        virtual void createTextureImgae2(VkFormat format);
        virtual void createTextureImageView(VkFormat format); ///< 텍스처 이미지 뷰 생성 함수
        virtual void createTextureSampler(); ///< 텍스처 샘플러 생성 함수

        VKcubeMap() = default;
    };
#endif
}
#endif // !INCLUDE_VKTEXTURE_H_
