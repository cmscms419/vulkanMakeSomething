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

            // frameIndex 는 update() 와 draw() 에 같은 값(Application 의 currentFrame)을 넘겨야 한다.
            void draw(const VkCommandBuffer cmd, cUint32_t frameIndex, VkImageView swapchainImageView, VkViewport viewport);
            void resize(cUint32_t width, cUint32_t height);

            bool update(cUint32_t frameIndex);
            PipelineHandle& getPipelineHandle();

        private:
            struct FrameGeometry
            {
                BaseBuffer2 vertexBuffer;
                BaseBuffer2 indexBuffer;
                cUint32_t vertexCapacity = 0; // 버퍼에 담을 수 있는 정점 수
                cUint32_t indexCapacity = 0;  // 버퍼에 담을 수 있는 인덱스 수

                explicit FrameGeometry(Context& ctx) : vertexBuffer(ctx), indexBuffer(ctx) {}
            };

            FrameGeometry& getFrameGeometry(cUint32_t frameIndex);

            Context& ctx;
            ShaderManager& shaderManager;

            std::vector<std::unique_ptr<FrameGeometry>> frameGeometries;

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
