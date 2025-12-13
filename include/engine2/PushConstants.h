#ifndef INCLUDE_PUSHCONSTANTS_H_
#define INCLUDE_PUSHCONSTANTS_H_

#include "common.h"
#include "struct.h"
#include "log.h"

#include "VKContext.h"

namespace vkengine {

    template <typename T>
    class PushConstants {

    public:
        PushConstants(VKcontext& context) : context(context)
        {

        }

        T& getData() {
            return this->data;
        }

        void Push(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
            vkCmdPushConstants(commandBuffer, pipelineLayout, stageFlags, 0, sizeof(T), &data);
        }

        size_t getSize() const {
            return sizeof(T);
        }
        
        VkPushConstantRange getPushConstantRange()
        {
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = stageFlags_;
            pushConstantRange.offset = 0;
            pushConstantRange.size = cUint32_t(size());
            return pushConstantRange;
        }

        void setStageFlags(VkShaderStageFlags stageFlags)
        {
            this->stageFlags = stageFlags;
        }

    private:
        VKcontext& context;
        
        VkShaderStageFlags stageFlags{ VK_SHADER_STAGE_ALL };
        T data{};
    };

}


#endif // !INCLUDE_PUSHCONSTANTS_H_
