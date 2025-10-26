#include "VKContext.h"
#include "VKImage2D.h"
#include "VKShaderManager.h"
#include "VKshader.h"
#include "VKpipeLineHandle.h"
#include "VKbarrier2.h"
#include "VKCommadBufferHander.h"
#include "VKbuffer2.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"


using namespace vkengine;

int main(int argc, char* argv[]) {

    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        std::cerr << "경로를 가져오는 데 실패했습니다." << std::endl;
    }

    std::vector<const char*> requiredInstanceExtensions = {};
    bool useSwapchain = false;

    VkContext ctx(requiredInstanceExtensions, useSwapchain);
    VkDevice device = ctx.getDevice()->logicaldevice;

    const uint32_t width = 640;
    const uint32_t height = 480;

    VKImage2D texture(ctx);
    texture.createImage(
        width,
        height,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, 
        1,
        1,
        (VkImageCreateFlagBits)0
    );

    VKShaderManager shaderManager(
        ctx,
        root_path + "../../../../../../shader/",
        {
            {"sample_pipeline", {"verttriangle.spv", "fragtriangle.spv"}}
        }
    );

    VKPipeLineHandle samplePipeline(ctx, shaderManager, "sample_pipeline",
        VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT);

    VKCommandBufferHander commandGrapicsHander = ctx.createGrapicsCommandBufferHander(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    texture.transitionToColorAttachment(commandGrapicsHander.getCommandBuffer());

    VkRenderingAttachmentInfo colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    colorAttachment.imageView = texture.getImageView(); 
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { 1.0f, 1.0f, 1.0f, 1.0f };

    VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    renderingInfo.renderArea = { {0, 0}, {width, height} };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = nullptr;
    renderingInfo.pStencilAttachment = nullptr;

    vkCmdBeginRendering(commandGrapicsHander.getCommandBuffer(), &renderingInfo);

    vkCmdBindPipeline(commandGrapicsHander.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
        *samplePipeline.getPipeline());

    VkViewport viewport{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height),
                        0.0f, 1.0f };
    vkCmdSetViewport(commandGrapicsHander.getCommandBuffer(), 0, 1, &viewport);

    VkRect2D scissor{ {0, 0}, {width, height} };
    vkCmdSetScissor(commandGrapicsHander.getCommandBuffer(), 0, 1, &scissor);

    vkCmdDraw(commandGrapicsHander.getCommandBuffer(), 3, 1, 0, 0);

    vkCmdEndRendering(commandGrapicsHander.getCommandBuffer());

    texture.transitionToTransferSrc(commandGrapicsHander.getCommandBuffer());

    commandGrapicsHander.submitAndWait();

    VkDeviceSize imageSize = width * height * 4;

    VkBaseBuffer2 statingBuffer;
    statingBuffer.device = device;
    statingBuffer.size = imageSize;
    statingBuffer.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    statingBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    statingBuffer.createBuffer(ctx.getDevice()->physicalDevice);
    statingBuffer.mapToMeBuffer(imageSize, 0);

    VKCommandBufferHander copyCmdBufferHander = ctx.createGrapicsCommandBufferHander(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    
    VkBufferImageCopy copyRegion{
        0, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {0, 0, 0}, {width, height, 1} };

    vkCmdCopyImageToBuffer(
        copyCmdBufferHander.getCommandBuffer(),
        texture.getImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        statingBuffer.buffer,
        1,
        &copyRegion
    );

    copyCmdBufferHander.submitAndWait();

    cUChar* data = static_cast<unsigned char*>(statingBuffer.mapped);
    cString outputImageFilename = "output.jpg";

    if (stbi_write_jpg(outputImageFilename.c_str(), width, height, 4, data, 90))
    {
        _PRINT_TO_CONSOLE_("이미지가 성공적으로 저장되었습니다.\n");
    }
    else
    {
        _PRINT_TO_CONSOLE_("이미지 저장에 실패했습니다.\n");
    }


    return 0;
}