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
        VkImage image = VK_NULL_HANDLE;                 // 텍스처 이미지 -> 텍스처 이미지를 저장하는 데 사용
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;    // 텍스처 이미지 메모리 -> 텍스처 이미지를 저장하는 데 사용
        VkImageView imageView = VK_NULL_HANDLE;         // 텍스처 이미지 뷰 -> 텍스처 이미지를 뷰로 변환 (이미지 뷰는 이미지를 읽고 쓰는 데 사용)
        VkSampler sampler = VK_NULL_HANDLE;             // 텍스처 샘플러 -> 텍스처 이미지를 샘플링하는 데 사용
        VkDescriptorImageInfo imageInfo{};              // 텍스처 이미지 정보 -> 텍스처 이미지를 설명하는 데 사용
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE; // 디스크립터 세트 -> 텍스처 이미지를 사용하는 셰이더에 전달되는 정보

        void createDescriptorImageInfo()
        {
            this->imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            this->imageInfo.imageView = this->imageView;
            this->imageInfo.sampler = this->sampler;
        };
    };

    struct imageResource {
        TextureResourcePNG* resourcePNG{};     // 리소스 정보 -> PNG 텍스처 리소스를 설명하는 데 사용
        TextureResourceKTX* resourceKTX{};     // KTX 리소스 정보 -> KTX 텍스처 리소스를 설명하는 데 사용
    };

    struct VkTextureBase {
    public:
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };  // 물리 디바이스 -> GPU Physical Handle
        VkDevice logicaldevice{ VK_NULL_HANDLE };           // 논리 디바이스 -> GPU Logical Handle
        VkCommandPool commandPool{ VK_NULL_HANDLE };        // 커맨드 풀 -> 커맨드 버퍼를 생성하는 데 사용
        VkQueue graphicsVKQueue{ VK_NULL_HANDLE };          // 그래픽스 큐 -> 그래픽스 명령을 처리하는 큐
        std::vector<VKimageData> imageData{};               // 텍스처 이미지 데이터 -> GPU에 보낼 데이터
        std::vector<imageResource> imageTextureDatas{};     // 텍스처 이미지 데이터 -> 리소스 데이터
        cUint32_t VKmipLevels = 1;                          // 텍스처의 Mip 레벨 수를 저장

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

        virtual void createTextureImagePNG() = 0; ///< 텍스처 이미지 생성 함수
        virtual void createTextureImgaeKTX(VkFormat format) { _PRINT_TO_CONSOLE_("CLEATE FUNTION"); };
        virtual void createTextureImageView(VkFormat format) = 0; ///< 텍스처 이미지 뷰 생성 함수
        virtual void createTextureSampler() = 0; ///< 텍스처 샘플러 생성 함수
        virtual void cleanResource()
        {
            for (auto& imageTextureData : this->imageTextureDatas)
            {
                if (imageTextureData.resourcePNG)
                {
                    delete imageTextureData.resourcePNG; // 리소스 해제
                }
                if (imageTextureData.resourceKTX)
                {
                    delete imageTextureData.resourceKTX; // 리소스 해제
                }
            }
            this->imageTextureDatas.clear();
        }

        virtual void cleanup() //< 텍스처 정리 함수
        {
            for (VKimageData& data : this->imageData)
            {
                vkDestroySampler(this->logicaldevice, data.sampler, nullptr);
                vkDestroyImageView(this->logicaldevice, data.imageView, nullptr);
                vkDestroyImage(this->logicaldevice, data.image, nullptr);
                vkFreeMemory(this->logicaldevice, data.imageMemory, nullptr);
            }
        }

        virtual void createDescriptorImageInfo() ///< 텍스처 이미지 정보 생성 함수
        {
            for (VKimageData& data : this->imageData) {
                data.createDescriptorImageInfo();
            }
        }
        
        void initializeDeviceHandles(VkPhysicalDevice physicalDevice, VkDevice logicaldevice, VkCommandPool commandPool, VkQueue graphicsVKQueue) ///< 디바이스 설정 함수
        {
            this->physicalDevice = physicalDevice;
            this->logicaldevice = logicaldevice;
            this->commandPool = commandPool;
            this->graphicsVKQueue = graphicsVKQueue;
        }

        VkTextureBase() = default; ///< 기본 생성자

    };

    struct Vk2DTexture : public VkTextureBase {
        virtual void createTextureImagePNG(); ///< 텍스처 이미지 생성 함수
        virtual void createTextureImgaeKTX(VkFormat format) {};
        virtual void createTextureImageView(VkFormat format); ///< 텍스처 이미지 뷰 생성 함수
        virtual void createTextureSampler(); ///< 텍스처 샘플러 생성 함수

        Vk2DTexture() = default;
    };

    struct Vk2DArrayTexture : public VkTextureBase {

        // https://github.com/SaschaWillems/Vulkan 이 사이트에서 texturearray 참고
        // 해당 예제는 리소스가 배열 형태로 되어 있어서, 배열 형태로 텍스처를 로드하는 예제이다.
        // 이 예제는 단일 리소스 여러개를 가져오는 형태로 여러번 리소스를 가져온다.
        virtual void createTextureImagePNG(); ///< 텍스처 이미지 생성 함수
        virtual void createTextureImgaeKTX(VkFormat format) {};
        virtual void createTextureImageView(VkFormat format); ///< 텍스처 이미지 뷰 생성 함수
        virtual void createTextureSampler(); ///< 텍스처 샘플러 생성 함수

        Vk2DArrayTexture() = default;
    private:
        cUint32_t imageCount = 0;   // 이미지 개수 -> 텍스처 이미지의 개수를 저장
    };

    struct VKcubeMap : public VkTextureBase {
        virtual void createTextureImagePNG(); ///< 텍스처 이미지 생성 함수
        virtual void createTextureImgaeKTX(VkFormat format);
        virtual void createTextureImageView(VkFormat format); ///< 텍스처 이미지 뷰 생성 함수
        virtual void createTextureSampler(); ///< 텍스처 샘플러 생성 함수
    
        VKcubeMap() = default;
    };
}
#endif // !INCLUDE_VKTEXTURE_H_
