#include "object3D.h"

#include "helper.h"
#include "VKengine.h"

namespace vkengine {
    namespace object {

        void Object::createVertexBuffer(std::vector<Vertex> vertices)
        {
            VkDevice logicaldevice = vkengine::VulkanEngine::Get().getDevice()->logicaldevice;
            VkPhysicalDevice physicalDevice = vkengine::VulkanEngine::Get().getDevice()->physicalDevice;
            VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

            this->vertexBuffer.device = logicaldevice;
            this->vertexBuffer.size = bufferSize;
            this->vertexBuffer.usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            this->vertexBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            this->vertexBuffer.createBuffer(physicalDevice);

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            helper::createBuffer(
                logicaldevice,
                physicalDevice,
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingBufferMemory);

            helper::copyToDeviceMemory(
                logicaldevice,
                vertices.data(),
                stagingBufferMemory,
                bufferSize);

            helper::copyBuffer2(
                *vkengine::VulkanEngine::Get().getDevice(),
                stagingBuffer,
                this->vertexBuffer.buffer,
                bufferSize);

            vkDestroyBuffer(logicaldevice, stagingBuffer, nullptr);
            vkFreeMemory(logicaldevice, stagingBufferMemory, nullptr);
        }

        void Object::createIndexBuffer(std::vector<uint16_t> indices)
        {
            VkDevice logicaldevice = vkengine::VulkanEngine::Get().getDevice()->logicaldevice;
            VkPhysicalDevice physicalDevice = vkengine::VulkanEngine::Get().getDevice()->physicalDevice;
            VkDeviceSize bufferSize = sizeof(uint16_t) * indices.size();
            this->indexBuffer.indexCount = static_cast<uint32_t>(indices.size());

            this->indexBuffer.device = logicaldevice;
            this->indexBuffer.size = bufferSize;
            this->indexBuffer.usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            this->indexBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            this->indexBuffer.createBuffer(physicalDevice);

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            helper::createBuffer(
                logicaldevice,
                physicalDevice,
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingBufferMemory);

            helper::copyToDeviceMemory(
                logicaldevice,
                indices.data(),
                stagingBufferMemory,
                bufferSize);

            helper::copyBuffer2(
                *vkengine::VulkanEngine::Get().getDevice(),
                stagingBuffer,
                this->indexBuffer.buffer,
                bufferSize);

            vkDestroyBuffer(logicaldevice, stagingBuffer, nullptr);
            vkFreeMemory(logicaldevice, stagingBufferMemory, nullptr);
        }

        void Object::draw(VkCommandBuffer commandBuffer, uint32_t imageIndex)
        {
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(commandBuffer, this->indexBuffer.indexCount, 1, 0, 0, 0);
        }

        void SkyBox::createCubeMap(std::vector<std::string> faces)
        {
            this->cubeMap.texWidth = 0;
            this->cubeMap.texHeight = 0;
            this->cubeMap.texChannels = 0;
            this->cubeMap.device = vkengine::VulkanEngine::Get().getDevice();

            this->cubeMap.createTextureCubeImages(4, faces);
            this->cubeMap.createTextureImageView(VK_FORMAT_R8G8B8A8_SRGB);
            this->cubeMap.createTextureSampler();
        };

        void TextureArrayObject3D::createCubeTexture(std::vector<std::string> faces)
        {
            this->cubeTextureArray.texWidth = 0;
            this->cubeTextureArray.texHeight = 0;
            this->cubeTextureArray.texChannels = 0;
            this->cubeTextureArray.device = vkengine::VulkanEngine::Get().getDevice();
            this->cubeTextureArray.VKmipLevels = 1;

            this->cubeTextureArray.createTextureArrayImages(4, faces);
            this->cubeTextureArray.createTextureImageView(VK_FORMAT_R8G8B8A8_SRGB);
            this->cubeTextureArray.createTextureSampler();
        }

    }
}

