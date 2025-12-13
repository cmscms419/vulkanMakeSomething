#ifndef VK_SHDAER_H_
#define VK_SHDAER_H_

#include "common.h"
#include "struct.h"
#include "log.h"

#include "VKcontext.h"

namespace vkengine {
    class VKshader
    {
        friend class VKShaderManager;

    public:
        VKshader(VKcontext& ctx, cString filepath);
        
        VKshader(VKshader&& other) noexcept;
        
        VKshader(const VKshader&) = delete;
        VKshader& operator=(const VKshader&) = delete;
        VKshader& operator=(VKshader&&) = delete;
        
        ~VKshader();
        
        void cleanup();

        
    private:
        VKcontext& ctx;
        VkShaderModule module{ VK_NULL_HANDLE };
        SpvReflectShaderModule reflectModule{};
        VkShaderStageFlagBits stage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };
        cString name{};

        std::vector<VkVertexInputAttributeDescription> makeVertexInputAttributeDescriptions() const;

    };
}


#endif // !VK_SHDAER_H_
