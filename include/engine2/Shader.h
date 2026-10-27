#ifndef VK_SHDAER_H_
#define VK_SHDAER_H_

#include "common.h"
#include "descriptor.h"

#include "Context.h"

namespace vkengine {
    class Shader
    {
        friend class ShaderManager;

    public:
        Shader(Context& ctx, cString filepath);
        Shader(Shader&& other) noexcept;
        
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader& operator=(Shader&&) = delete;
        
        ~Shader();
        
        void cleanup();

        
    private:
        Context& ctx;
        VkShaderModule module;
        SpvReflectShaderModule reflectModule;
        VkShaderStageFlagBits stage;
        cString name;

        std::vector<VkVertexInputAttributeDescription> makeVertexInputAttributeDescriptions() const;
        std::vector<VertexInputinfo> makeVertexInputInfo() const;

    };
}

#endif // !VK_SHDAER_H_
