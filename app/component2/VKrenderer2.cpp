#include "VKrenderer2.h"

#include "log.h"
#include "resourseload.h"

using namespace vkengine::Log;

namespace vkengine
{

    VKforwardRenderer2::VKforwardRenderer2(VKcontext &ctx, VKShaderManager &shadermanager, const cUint32_t &MaxFramesFlight, const cString &assetsPath, const cString &shaderPath)
    {
    }

    VKforwardRenderer2::~VKforwardRenderer2()
    {
        this->cleanup();
    }

    void VKforwardRenderer2::cleanup()
    {
    }

    void VKforwardRenderer2::executeShadowPass(VkCommandBuffer cmd, cUint32_t frameIndex)
    {
        if (this->shadowMap.getImage() == VK_NULL_HANDLE)
        {
            return; // Shadow map not initialized
        }

        if (this->currentModels->empty())
        {
            return; // No currentModels set for shadow pass
        }

        this->makeShadowMap(cmd, frameIndex);
    }

    void VKforwardRenderer2::executeForwardPass(VkCommandBuffer cmd, cUint32_t frameIndex)
    {
    }

    void VKforwardRenderer2::executePostPass(VkCommandBuffer cmd, cUint32_t frameIndex)
    {
    }

    void VKforwardRenderer2::makeShadowMap(VkCommandBuffer cmd, uint32_t currentFrame)
    {
#if 1
        // 그림자 맵 렌더링 시작
        VkRenderingAttachmentInfo shadowDepthAttachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
        shadowDepthAttachment.imageView = this->shadowMap.getImageView();
        shadowDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        shadowDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        shadowDepthAttachment.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo shadowRenderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
        shadowRenderingInfo.renderArea = {0, 0, this->shadowMap.getWidth(), this->shadowMap.getHeight()};
        shadowRenderingInfo.layerCount = 1;
        shadowRenderingInfo.colorAttachmentCount = 0;
        shadowRenderingInfo.pDepthAttachment = &shadowDepthAttachment;

        VkViewport shadowViewport{0.0f, 0.0f, (float)this->shadowMap.getWidth(), (float)this->shadowMap.getHeight(),
                                  0.0f, 1.0f};
        VkRect2D shadowScissor{0, 0, this->shadowMap.getWidth(), this->shadowMap.getHeight()};

        vkCmdBeginRendering(cmd, &shadowRenderingInfo);
        vkCmdSetViewport(cmd, 0, 1, &shadowViewport);
        vkCmdSetScissor(cmd, 0, 1, &shadowScissor);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines.at("shadowMap").getPipeline());

        const auto descriptorSets = std::vector{this->SceneOptionsBoneDataSets[currentFrame].get()};

        vkCmdBindDescriptorSets(
            cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines.at("shadowMap").getPipelineLayout(), 0,
            static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

        vkCmdSetDepthBias(cmd,
                          1.1f,  // Constant factor
                          0.0f,  // Clamp value
                          2.0f); // Slope factor

        // Render all visible currentModels to shadow map
        VkDeviceSize offsets[1]{0};

        for (size_t j = 0; j < currentModels->size(); j++)
        {
            if (!currentModels->at(j).Visible())
            {
                continue;
            }

            vkCmdPushConstants(cmd, this->pipelines.at("shadowMap").getPipelineLayout(),
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(currentModels->at(j).ModelMatrix()),
                               &currentModels->at(j).ModelMatrix());

            // Render all meshes in this model
            for (size_t i = 0; i < currentModels->at(j).Meshes().size(); i++)
            {
                auto &mesh = currentModels->at(j).Meshes()[i];

                // Skip culled meshes in shadow pass too
                if (mesh.isCulled)
                {
                    continue;
                }

                // Bind vertex and index buffers
                vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertex->Buffer(), offsets);
                vkCmdBindIndexBuffer(cmd, mesh.index->Buffer(), 0, VK_INDEX_TYPE_UINT32);

                // Draw the mesh
                vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
            }
        }

        vkCmdEndRendering(cmd);
#else

#endif
    }

}