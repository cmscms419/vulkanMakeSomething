#ifndef VK_SHDAER_H_
#define VK_SHDAER_H_

#include "common.h"

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
        VkShaderModule module;
        SpvReflectShaderModule reflectModule;
        VkShaderStageFlagBits stage;
        cString name;

        std::vector<VkVertexInputAttributeDescription> makeVertexInputAttributeDescriptions() const;

    };
}


#endif // !VK_SHDAER_H_
