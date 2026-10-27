#ifndef INCLUDE_PUSHCONSTANTS_H_
#define INCLUDE_PUSHCONSTANTS_H_

#include "common.h"
#include "log.h"

#include "Context.h"

namespace vkengine {

    template <typename T>
    class PushConstants {

    public:
        PushConstants(Context& context) : context(context)
        {
            VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL;
            T data = {};
        }

        T& getData() {
            return this->data;
        }

        void push(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
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
        Context& context;
        VkShaderStageFlags stageFlags;
        T data;
    };

}

#endif // !INCLUDE_PUSHCONSTANTS_H_
