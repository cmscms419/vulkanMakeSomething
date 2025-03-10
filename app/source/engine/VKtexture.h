#ifndef INCLUDE_VKTEXTURE_H_
#define INCLUDE_VKTEXTURE_H_

#include "../../common/common.h"
#include "../../common/struct.h"
#include "../../common/macros.h"

#include "VKdevice.h"

struct VkTexture_ {

    vkengine::VKDevice_* device = VK_NULL_HANDLE;                   // Vulkan 장치 핸들
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;   // Vulkan 물리 장치 핸들
    VkImage image = VK_NULL_HANDLE;                 // 텍스처 이미지 -> 텍스처 이미지를 저장하는 데 사용
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;    // 텍스처 이미지 메모리 -> 텍스처 이미지를 저장하는 데 사용
    VkImageView imageView = VK_NULL_HANDLE;         // 텍스처 이미지 뷰 -> 텍스처 이미지를 뷰로 변환 (이미지 뷰는 이미지를 읽고 쓰는 데 사용)
    VkSampler sampler = VK_NULL_HANDLE;             // 텍스처 샘플러 -> 텍스처 이미지를 샘플링하는 데 사용
    uint32_t VKmipLevels;                               // 밉 레벨 -> 이미지의 미입 레벨

    int texWidth = 0;                         ///< 텍스처 너비
    int texHeight = 0;                        ///< 텍스처 높이
    uint32_t texChannels = 0;                      ///< 텍스처 채널
    std::string texPath = "";                      ///< 텍스처 경로

    //VkDevice device = VK_NULL_HANDLE; ///< Vulkan 장치 핸들
    //VkDescriptorImageInfo descriptor{}; ///< Vulkan 디스크립터 이미지 정보
    //VkFormat format; ///< 이미지 포맷
    //VkExtent3D extent{}; ///< 이미지 크기
    //uint32_t mipLevels; ///< 이미지 미플레벨
    //uint32_t layerCount; ///< 이미지 레이어 수
    //VkImageLayout layout; ///< 이미지 레이아웃
    //VkImageAspectFlags aspectFlags; ///< 이미지 액세스 플래그
    //VkImageUsageFlags usageFlags; ///< 이미지 사용 플래그
    //VkMemoryPropertyFlags memoryPropertyFlags; ///< 메모리 속성 플래그
    //VkSamplerAddressMode addressMode; ///< 샘플러 주소 모드
    //VkFilter filter; ///< 샘플러 필터
    //VkSamplerMipmapMode mipmapMode; ///< 샘플러 미플 모드
    //VkComponentMapping componentMapping{}; ///< 컴포넌트 매핑
    //VkImageSubresourceRange subresourceRange{}; ///< 이미지 서브리소스 레인지
    //VkImageSubresourceLayers subresourceLayers{}; ///< 이미지 서브리소스 레이어
    //VkImageCreateInfo imageInfo{}; ///< 이미지 생성 정보
    //VkImageViewCreateInfo viewInfo{}; ///< 이미지 뷰 생성 정보
    //VkSamplerCreateInfo samplerInfo{}; ///< 샘플러 생성 정보
    //VkDescriptorImageInfo descriptorInfo{}; ///< 디스크립터 이미지 정보
    //VkImageSubresource subresource{}; ///< 이미지 서브리소스

    void cleanup(); ///< 텍스처 정리 함수

};

struct Vk2DTexture_ : public VkTexture_ {

    void createTextureImage(int type); ///< 텍스처 이미지 생성 함수
    void createTextureImageView(VkFormat format); ///< 텍스처 이미지 뷰 생성 함수
    void createTextureSampler(); ///< 텍스처 샘플러 생성 함수
};


#endif // !INCLUDE_VKTEXTURE_H_
