#ifndef INCLUDE_VK_SHADERRESOURCE_H_
#define INCLUDE_VK_SHADERRESOURCE_H_

#include "common.h"

#include "VKbarrier2.h"

namespace vkengine
{
    enum shaderResourceType
    {
        NONE,
        IMAGE,
        VERTEX_BUFFER,
        INDEX_BUFFER,
        UNIFORM_BUFFER
    };

    class VKShaderResource
    {
    public:
        virtual ~VKShaderResource() = default;

        // Descriptor Set Layout 정보 — descriptorType, descriptorCount 기입
        // stageFlags는 ShaderManager가 SPIRV-Reflect로 결정해서 주입함
        virtual void updateBinding(VkDescriptorSetLayoutBinding &binding);

        // 실제 GPU 리소스 데이터 — bufferInfo 또는 imageInfo 채우기
        // dstSet / dstBinding 은 DescriptorSetHander::create() 가 덮어씀
        virtual void updateWrite(VkWriteDescriptorSet &write);

        virtual void cleanup() = 0;

        void update();
        void setSampler(VkSampler sampler);
        VKBarrierHelper &getBarrierHelper();

    protected:
        cString name;

        VkImage image{VK_NULL_HANDLE};
        VkImageView imageView{VK_NULL_HANDLE};
        VkImageLayout imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};
        VkSampler sampler{VK_NULL_HANDLE};

        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceSize bufferSize{0};
        void *mapped; //< 매핑된 메모리 포인터
        
        VkDeviceMemory memory;
        VkDescriptorType descriptorType{};
        cUint32_t descriptorCount{};
        VkShaderStageFlags stageFlags{};

        VkDescriptorImageInfo imageInfo{};
        VkDescriptorBufferInfo bufferInfo{};
        VkBufferView texelBufferView = {VK_NULL_HANDLE};


        VKBarrierHelper barrierHelper;
    };
}

#endif // INCLUDE_VK_SHADERRESOURCE_H_