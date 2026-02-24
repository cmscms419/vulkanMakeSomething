#ifndef INCLUDE_HELPER_SHADER_H_
#define INCLUDE_HELPER_SHADER_H_

#include "common.h"
#include "log.h"

namespace vkengine
{
    namespace helper
    {
        namespace shader
        {
            SpvReflectShaderModule createSpvReflectModule(const std::vector<cChar> &code);

            VkShaderModule createShaderModule(VkDevice device, const std::vector<cChar> &code);

            const cChar *getShaderStageString(const SpvReflectShaderStageFlagBits &stage);

            cString shaderStageFlagsToString(VkShaderStageFlags flags);

            void printReflectionInfo(const SpvReflectShaderModule &reflectModule);

            VkFormat getVkFormatFromSpvReflectFormat(SpvReflectFormat format);

            cUint32_t getFormatSize(VkFormat format);

        }
    }
}

#endif // INCLUDE_HELPER_SHADER_H_