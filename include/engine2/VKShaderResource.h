#ifndef INCLUDE_VK_SHADERRESOURCE_H_
#define INCLUDE_VK_SHADERRESOURCE_H_

#include "common.h"

#include "VKbarrier2.h"

namespace vkengine
{
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

        virtual void cleanup();
        virtual void update();

    protected:
        cString name;

        VkDeviceMemory memory{VK_NULL_HANDLE};
        VkDescriptorType descriptorType{};
        cUint32_t descriptorCount{};
        VkShaderStageFlags stageFlags{};

        VkDescriptorImageInfo imageInfo{};
        VkDescriptorBufferInfo bufferInfo{};
    };

    class VKBufferShaderResource : public VKShaderResource
    {
    public:
        virtual void update() override;
        virtual void updateWrite(VkWriteDescriptorSet &write) override;
        
        void updateBufferInfo(VkDescriptorBufferInfo &bufferInfo);

    protected:
        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceSize bufferSize{0};
        VkDeviceSize offset;        ///< 버퍼 간격
        VkDeviceSize allocatedSize; ///< createBuffer 할 때, 만들어지는 버퍼의 크기
        VkDeviceSize alignment;     ///< 버퍼 정렬
        void *mapped;               //< 매핑된 메모리 포인터
    };

    class VKImageShaderResource : public VKShaderResource
    {
    public:
        virtual void update() override;
        virtual void updateWrite(VkWriteDescriptorSet &write) override;
        
        void setSampler(VkSampler sampler);
        void updateImageInfo(VkDescriptorImageInfo &imageInfo);

        VKBarrierHelper &getBarrierHelper();

        void transitionTo(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkAccessFlags2 newAccess, VkPipelineStageFlags2 newStage);
        void transitionToColorAttachment(VkCommandBuffer commandBuffer);
        void transitionToDepthStencilAttachment(VkCommandBuffer commandBuffer);
        void transitionToTransferDst(VkCommandBuffer commandBuffer);
        void transitionToTransferSrc(VkCommandBuffer commandBuffer);
        void transitionToShaderReadOnly(VkCommandBuffer commandBuffer);
        void transitionToShaderReadWrite(VkCommandBuffer commandBuffer);
        void transitionToShaderWriteOnly(VkCommandBuffer commandBuffer);
        void transitionToPresent(VkCommandBuffer commandBuffer);
        
        void updateResourceBindingAfterTransition();

    protected:
        VkImage image{VK_NULL_HANDLE};
        VkImageView imageView{VK_NULL_HANDLE};
        VkSampler sampler{VK_NULL_HANDLE};

        VKBarrierHelper barrierHelper;
    };
}

#endif // INCLUDE_VK_SHADERRESOURCE_H_