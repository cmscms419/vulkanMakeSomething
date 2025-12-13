#ifndef INCLUDE_VK_GUI_H_
#define INCLUDE_VK_GUI_H_

#include "common.h"
#include "log.h"

#include "VKContext.h"
#include "VKbuffer2.h"
#include "VKImage2D.h"
#include "VKsampler.h"
#include "VKShaderManager.h"
#include "VKDescriptorManager2.h"
#include "DescriptorSet.h"
#include "pipeLineHandle.h"
#include "PushConstants.h"

#include <imgui.h>

namespace vkengine {
    namespace gui {

        struct PushConstBlock
        {
            glm::vec2 scale{ 1.0f, 1.0f };
            glm::vec2 translate{ 0.0f, 0.0f };
        };

        class VKimguiRenderer {
        public:
            VKimguiRenderer(VKcontext& context, VKShaderManager& shaderManager, VkFormat colorFormat);
            ~VKimguiRenderer();

            void draw(const VkCommandBuffer cmd, VkImageView swapchainImageView, VkViewport viewport);
            void resize(uint32_t width, uint32_t height);

            bool update();
            PipeLineHandle& imguiPipeLine();

        private:
            VKcontext& ctx;
            VKShaderManager& shaderManager;

            VKBaseBuffer2 VertexBuffer;
            VKBaseBuffer2 IndexBuffer;
            cUint32_t vertexCount = 0;
            cUint32_t indexCount = 0;

            VKImage2D fontImage;
            VKSampler fontSampler;
            PipeLineHandle pipelineHandle;

            DescriptorSetHander fontSet;
            PushConstants<PushConstBlock> pushConsts;

            bool visible{ true };
            bool updated{ false };
            float scale{ 1.4f };
            float updateTimer{ 0.0f };

        };
} // namespace gui
} // namespace vkengine

#endif // !INCLUDE_VK_GUI_H_
