#ifndef INCLUDE_VK_SHADERRESOURCE_H_
#define INCLUDE_VK_SHADERRESOURCE_H_

#include "common.h"

#include "VKResourceBindingData.h"

namespace vkengine
{
    class VKShaderResource
    {
    public:
        virtual ~VKShaderResource() = default;

        // [1] Descriptor Set Layout 정보 — descriptorType, descriptorCount 기입
        //     stageFlags는 ShaderManager가 SPIRV-Reflect로 결정해서 주입함
        virtual void updateBinding(VkDescriptorSetLayoutBinding &binding) = 0;

        // [2] 실제 GPU 리소스 데이터 — bufferInfo 또는 imageInfo 채우기
        //     dstSet / dstBinding 은 DescriptorSetHander::create() 가 덮어씀
        virtual void updateWrite(VkWriteDescriptorSet &write) = 0;

        // [3] BarrierHelper 접근 (RenderGraph 배리어 삽입용)
        virtual VKResourceBinding &getResourceBinding() = 0;
        virtual const VKResourceBinding &getResourceBinding() const = 0;
    };
}

#endif // INCLUDE_VK_SHADERRESOURCE_H_