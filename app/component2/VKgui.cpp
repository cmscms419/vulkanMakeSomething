#include "VKgui.h"

using namespace vkengine::Log;

namespace vkengine {
    namespace gui {
        
        VKimguiRenderer::VKimguiRenderer(VKcontext& context, VKShaderManager& shaderManager, VkFormat colorFormat, std::string path)
            : ctx(context), shaderManager(shaderManager), VertexBuffer(context), IndexBuffer(context),
            fontImage(context), fontSampler(context), pushConsts(context), pipelineHandle(context, shaderManager)
        {
            pipelineHandle.createByName("gui", colorFormat);
            pushConsts.setStageFlags(VK_SHADER_STAGE_VERTEX_BIT);

            // ImGUI 초기화 설정
            ImGui::CreateContext();
            ImGuiStyle& style = ImGui::GetStyle();
            style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
            style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
            style.ScaleAllSizes(this->scale);
            ImGuiIO& io = ImGui::GetIO();
            io.FontGlobalScale = this->scale;

            {
                // 폰트 경로를 재수정 해야한다.
                const cString fontFileName =
                    path + "/noGit/Noto_Sans_KR/static/NotoSansKR_SemiBold.ttf"; // Korean Font

                unsigned char* fontData = nullptr;
                int texWidth, texHeight;
                ImGuiIO& io = ImGui::GetIO();

                ImFontConfig config;
                config.MergeMode = false;
                
                io.Fonts->AddFontFromFileTTF(fontFileName.c_str(), 16.0f * this->scale, &config,
                    io.Fonts->GetGlyphRangesDefault());

                config.MergeMode = true;
                config.OversampleH = 2;  // 폰트 품질 개선
                config.OversampleV = 1;

                io.Fonts->AddFontFromFileTTF(fontFileName.c_str(), 16.0f * this->scale, &config,
                    io.Fonts->GetGlyphRangesKorean());

                io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

                if (!fontData)
                {
                    EXIT_TO_LOGGER("Failed to load font data from : %s", fontFileName);
                }
                
                this->fontImage.createTextureFromPixelData(fontData, texWidth, texHeight, 4, false);
            }

            fontSampler.createAnisoRepeat();
            fontImage.setSampler(fontSampler.getSampler());
            fontSet.create(this->ctx, { std::ref(fontImage.getResourceBinding()) });
        }

        VKimguiRenderer::~VKimguiRenderer()
        {
            if (ImGui::GetCurrentContext())
            {
                ImGui::DestroyContext();
            }
        }
        
        void VKimguiRenderer::draw(const VkCommandBuffer cmd, VkImageView swapchainImageView, VkViewport viewport)
        {
            VkRenderingAttachmentInfo swapchainColorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
            swapchainColorAttachment.imageView = swapchainImageView;
            swapchainColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            swapchainColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // Preserve previous content
            swapchainColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            VkRenderingInfo colorOnlyRenderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
            colorOnlyRenderingInfo.renderArea = { 0, 0, cUint32_t(viewport.width), cUint32_t(viewport.height) };
            colorOnlyRenderingInfo.layerCount = 1;
            colorOnlyRenderingInfo.colorAttachmentCount = 1;
            colorOnlyRenderingInfo.pColorAttachments = &swapchainColorAttachment;

            ImDrawData* imDrawData = ImGui::GetDrawData();
            int32_t vertexOffset = 0;
            int32_t indexOffset = 0;

            if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
                return;
            }

            vkCmdBeginRendering(cmd, &colorOnlyRenderingInfo); 

            vkCmdSetViewport(cmd, 0, 1, &viewport);

            const auto temp = std::vector{ fontSet.get()};
            
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineHandle.getPipeline());
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineHandle.getPipelineLayout(), 0,
                cUint32_t(temp.size()), temp.data(), 0, NULL);

            ImGuiIO& io = ImGui::GetIO();
            auto& pc = this->pushConsts.getData();
            pc.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
            pc.translate = glm::vec2(-1.0f);
            pushConsts.Push(cmd, this->pipelineHandle.getPipelineLayout());

            VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(cmd, 0, 1, &this->VertexBuffer.Buffer(), offsets);
            vkCmdBindIndexBuffer(cmd, this->IndexBuffer.Buffer(), 0, VK_INDEX_TYPE_UINT16);

            for (cInt32_t i = 0; i < imDrawData->CmdListsCount; i++) {
                const ImDrawList* cmd_list = imDrawData->CmdLists[i];
                for (cInt32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
                    VkRect2D scissorRect;
                    
                    scissorRect.offset.x = std::max((cInt32_t)(pcmd->ClipRect.x), 0);
                    scissorRect.offset.y = std::max((cInt32_t)(pcmd->ClipRect.y), 0);
                    
                    scissorRect.extent.width = (cUint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                    scissorRect.extent.height = (cUint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
                    
                    vkCmdSetScissor(cmd, 0, 1, &scissorRect);
                    vkCmdDrawIndexed(cmd, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
                    
                    indexOffset += pcmd->ElemCount;
                }

                vertexOffset += cmd_list->VtxBuffer.Size;
            }

            vkCmdEndRendering(cmd);
        }

        void VKimguiRenderer::resize(cUint32_t width, cUint32_t height)
        {
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2((float)(width), (float)(height));
        }
        
        bool VKimguiRenderer::update()
        {
            ImDrawData* imDrawData = ImGui::GetDrawData();
            bool updateCmdBuffers = false;

            if (!imDrawData) {
                return false;
            }

            VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
            VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

            if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
                return false;
            }

            if ((this->VertexBuffer.Buffer() == VK_NULL_HANDLE) ||
                (imDrawData->TotalVtxCount > cInt(this->vertexCount)))
            {
                this->ctx.waitGraphicsQueueIdle();
                this->VertexBuffer.createVertexBuffer(vertexBufferSize, nullptr);
                this->vertexCount = imDrawData->TotalVtxCount; // 버퍼 용량을 의미한다
                updateCmdBuffers = true;
            }

            if ((this->IndexBuffer.Buffer() == VK_NULL_HANDLE) ||
                (imDrawData->TotalIdxCount > cInt(this->indexCount)))
            {
                this->ctx.waitGraphicsQueueIdle();
                this->IndexBuffer.createIndexBuffer(indexBufferSize, nullptr);
                this->indexCount = imDrawData->TotalIdxCount; // 버퍼 용량을 의미한다
                updateCmdBuffers = true;
            }

            ImDrawVert* vtxDst = (ImDrawVert*)VertexBuffer.Mapped();
            ImDrawIdx* idxDst = (ImDrawIdx*)IndexBuffer.Mapped();

            for (int n = 0; n < imDrawData->CmdListsCount; n++) {
                const ImDrawList* cmd_list = imDrawData->CmdLists[n];
                memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtxDst += cmd_list->VtxBuffer.Size;
                idxDst += cmd_list->IdxBuffer.Size;
            }

            VertexBuffer.flush();
            IndexBuffer.flush();

            return updateCmdBuffers;
        }
        
        PipeLineHandle& VKimguiRenderer::imguiPipeLine()
        {
            return this->pipelineHandle;
        }
    }
}

