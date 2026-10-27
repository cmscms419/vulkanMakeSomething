#ifndef INCLUDE_VK_GUI_H_
#define INCLUDE_VK_GUI_H_

#include "common.h"
#include "log.h"

#include "Context.h"
#include "BaseBuffer2.h"
#include "Image2D.h"
#include "SamplerHandle.h"
#include "ShaderManager.h"
#include "DescriptorManager2.h"
#include "DescriptorSetHandle.h"
#include "PipelineHandle.h"
#include "PushConstants.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace vkengine {
    namespace gui {

        struct PushConstBlock
        {
            glm::vec2 scale{ 1.0f, 1.0f };
            glm::vec2 translate{ 0.0f, 0.0f };
        };

        class ImGuiRenderer {
        public:
            ImGuiRenderer(Context& context, ShaderManager& shaderManager, VkFormat colorFormat, std::string path);
            ~ImGuiRenderer();

            void draw(const VkCommandBuffer cmd, VkImageView swapchainImageView, VkViewport viewport);
            void resize(cUint32_t width, cUint32_t height);

            bool update();
            PipelineHandle& getPipelineHandle();

        private:
            Context& ctx;
            ShaderManager& shaderManager;

            BaseBuffer2 VertexBuffer;
            BaseBuffer2 IndexBuffer;
            cUint32_t vertexCount = 0;
            cUint32_t indexCount = 0;

            Image2D fontImage;
            SamplerHandle fontSampler;
            PipelineHandle pipelineHandle;

            DescriptorSetHandle fontSet;
            PushConstants<PushConstBlock> pushConsts;

            bool visible{ true };
            bool updated{ false };
            float scale{ 1.4f };
            float updateTimer{ 0.0f };

        };
} // namespace gui
} // namespace vkengine

#endif // !INCLUDE_VK_GUI_H_
