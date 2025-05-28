#include "object3D.h"

namespace vkengine {
    namespace object {

        void Object::createVertexBuffer(std::vector<Vertex>& vertices)
        {
            if (vertices.empty() && !this->getVertices()->empty())
            {
                vertices = *this->getVertices();
            }

            if (vertices.empty())
            {
                _PRINT_TO_CONSOLE_("Vertex buffer is empty\n");
                return;
            }

            VKDevice_* device = vkengine::VulkanEngine::Get().getDevice();
            VkDevice logicaldevice = device->logicaldevice;
            VkPhysicalDevice physicalDevice = device->physicalDevice;
            VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

            this->vertexBuffer.vertices = vertices;
            this->vertexBuffer.device = logicaldevice;
            this->vertexBuffer.size = bufferSize;
            this->vertexBuffer.usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            this->vertexBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            this->vertexBuffer.createBuffer(physicalDevice);

            VertexBuffer staging;
            staging.device = logicaldevice;
            staging.size = bufferSize;
            staging.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            staging.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            staging.createBuffer(physicalDevice);

            staging.mapToMeBuffer(bufferSize, 0);
            staging.copyToMeBuffer(vertices.data(), bufferSize);

            helper::copyBuffer2(*device, staging.buffer, this->vertexBuffer.buffer, bufferSize);

            staging.cleanup();
        }

        void Object::createIndexBuffer(std::vector<uint16_t>& indices)
        {
            if (indices.empty() && !this->getIndices()->empty())
            {
                indices = *this->getIndices();
            }

            if (indices.empty())
            {
                _PRINT_TO_CONSOLE_("Index buffer is empty\n");
                return;
            }

            VKDevice_* device = vkengine::VulkanEngine::Get().getDevice();
            VkDevice logicaldevice = device->logicaldevice;
            VkPhysicalDevice physicalDevice = device->physicalDevice;
            VkDeviceSize bufferSize = sizeof(uint16_t) * indices.size();

            this->indexBuffer.indices = indices;
            this->indexBuffer.device = logicaldevice;
            this->indexBuffer.size = bufferSize;
            this->indexBuffer.usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            this->indexBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            this->indexBuffer.createBuffer(physicalDevice);

            IndexBuffer staging;
            staging.device = logicaldevice;
            staging.size = bufferSize;
            staging.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            staging.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            staging.createBuffer(physicalDevice);

            staging.mapToMeBuffer(bufferSize, 0);
            staging.copyToMeBuffer(indices.data(), bufferSize);

            helper::copyBuffer2(*device, staging.buffer, this->indexBuffer.buffer, bufferSize);

            staging.cleanup();
        }

        void Object::draw(VkCommandBuffer commandBuffer, uint32_t imageIndex)
        {
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(this->indexBuffer.indices.size()), 1, 0, 0, 0);
        }

        void Object::RotationAngle(float angle, glm::vec3 axis)
        {
            this->rotation = glm::angleAxis(glm::radians(angle), axis);
        }

        void Object::updateMatrix()
        {
            glm::mat4 updateMatrix = glm::mat4(1.0f);

            glm::mat4 T = glm::translate(glm::mat4(1.0f), this->position);
            glm::mat4 R = vkMath::fromQuatToMatrix(this->rotation);
            glm::mat4 S = glm::scale(glm::mat4(1.0f), this->scale);
            updateMatrix = updateMatrix * T * R * S;

            this->matrix = updateMatrix;
        }

        void Object::createModelViewProjBuffers()
        {

            VKDevice_* device = vkengine::VulkanEngine::Get().getDevice();
            VkDevice logicaldevice = device->logicaldevice;
            VkPhysicalDevice physicalDevice = device->physicalDevice;
            VkDeviceSize bufferSize = sizeof(UniformBufferObject);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                this->modelviewprojUniformBuffer[i].device = logicaldevice;
                this->modelviewprojUniformBuffer[i].size = sizeof(UniformBufferObject);
                this->modelviewprojUniformBuffer[i].usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                this->modelviewprojUniformBuffer[i].memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

                this->modelviewprojUniformBuffer[i].createBuffer(physicalDevice);
                this->modelviewprojUniformBuffer[i].mapToMeBuffer(bufferSize, 0);
            }

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

        void SkyBox::updateUniformBuffer(uint32_t currentImage, Camera* camera)
        {
            UniformBufferObject ubo{};
            ubo.model = glm::mat4(1.0f);
            ubo.view = camera->getViewMatrix();
            ubo.proj = camera->getProjectionMatrix();
            memcpy(this->modelviewprojUniformBuffer[currentImage].mapped, &ubo, sizeof(ubo));
        }

        void TextureArrayObject3D::createTextureArray(std::vector<std::string> faces)
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

        void Texture3DObject::createTexture(std::string path)
        {
            this->texture.texWidth = 0;
            this->texture.texHeight = 0;
            this->texture.texChannels = 0;

            this->texture.device = vkengine::VulkanEngine::Get().getDevice();
            this->texture.VKmipLevels = 1;

            this->texture.createTextureImage(4, path.c_str());
            this->texture.createTextureImageView(VK_FORMAT_R8G8B8A8_SRGB);
            this->texture.createTextureSampler();
        }

        void ModelObject::createTexture(std::string path)
        {
            this->texture.texWidth = 0;
            this->texture.texHeight = 0;
            this->texture.texChannels = 0;
            this->texture.device = vkengine::VulkanEngine::Get().getDevice();

            this->texture.createTextureImage(4, path.c_str());
            this->texture.createTextureImageView(VK_FORMAT_R8G8B8A8_SRGB);
            this->texture.createTextureSampler();
        }

        void ModelObject::updateUniformBuffer(uint32_t currentImage, glm::mat4 world, Camera* camera)
        {
            UniformBufferObject ubo{};

            ubo.model = world * this->matrix;
            ubo.view = camera->getViewMatrix();
            ubo.proj = camera->getProjectionMatrix();

            memcpy(this->modelviewprojUniformBuffer[currentImage].mapped, &ubo, sizeof(ubo));
        }

    }
}

