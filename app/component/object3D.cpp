#include "object3D.h"

namespace vkengine {
    namespace object {

        void Object3d::createVertexBuffer(std::vector<Vertex>& vertices)
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

            VkDevice logicaldevice = this->logicaldevice;
            VkPhysicalDevice physicalDevice = this->physicalDevice;
            VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

            this->vertices = vertices;
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

            helper::copyBuffer(this->logicaldevice, this->commandPool, this->graphicsVKQueue, staging.buffer, this->vertexBuffer.buffer, bufferSize);

            staging.cleanup();
        }

        void Object3d::createIndexBuffer(std::vector<cUint32_t>& indices)
        {
            if (indices.empty() && !this->getIndices()->empty())
            {
                indices = this->indices;
            }

            if (indices.empty())
            {
                _PRINT_TO_CONSOLE_("Index buffer is empty\n");
                return;
            }

            VkDevice logicaldevice = this->logicaldevice;
            VkPhysicalDevice physicalDevice = this->physicalDevice;
            VkDeviceSize bufferSize = sizeof(cUint32_t) * indices.size();

            this->indices = indices;
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

            helper::copyBuffer(this->logicaldevice, this->commandPool, this->graphicsVKQueue, staging.buffer, this->indexBuffer.buffer, bufferSize);

            staging.cleanup();
        }

        void Object3d::draw(VkCommandBuffer commandBuffer, bool drawIndex)
        {
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &this->vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

            if (drawIndex)
            {
                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(this->indices.size()), 1, 0, 0, 0);
            }
        }

        void Object3d::RotationAngle(float angle, cVec3 axis)
        {
            this->rotation = glm::angleAxis(glm::radians(angle), axis);
        }

        void Object3d::updateMatrix()
        {
            cMat4 updateMatrix = cMat4(1.0f);

            cMat4 T = glm::translate(cMat4(1.0f), this->position);
            cMat4 R = vkMath::fromQuatToMatrix(this->rotation);
            cMat4 S = glm::scale(cMat4(1.0f), this->scale);
            updateMatrix = updateMatrix * T * R * S;

            this->matrix = updateMatrix;
        }

        void Object3d::createModelViewProjBuffers()
        {

            VkDevice logicaldevice = this->logicaldevice;
            VkPhysicalDevice physicalDevice = this->physicalDevice;
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

        void SkyBox::updateUniformBuffer(uint32_t currentImage, Camera* camera)
        {
            UniformBufferObject ubo{};
            ubo.model = cMat4(1.0f);
            ubo.view = camera->getViewMatrix();
            ubo.proj = camera->getProjectionMatrix();
            ubo.inverseTranspose = cMat4(1.0f); // skymap은 역행렬이 필요없음

            memcpy(this->modelviewprojUniformBuffer[currentImage].mapped, &ubo, sizeof(ubo));
        }

        void ModelObject::updateUniformBuffer(uint32_t currentImage, cMat4 world, Camera* camera)
        {
            UniformBufferObject ubo{};

            ubo.model = world * this->matrix;
            ubo.view = camera->getViewMatrix();
            ubo.proj = camera->getProjectionMatrix();
            ubo.inverseTranspose = glm::transpose(glm::inverse(this->matrix)); // 역행렬의 전치 행렬

            memcpy(this->modelviewprojUniformBuffer[currentImage].mapped, &ubo, sizeof(ubo));
        }

        void ParticleObject::createParticleBuffers(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
        {
            VkDevice logicaldevice = this->logicaldevice;
            VkPhysicalDevice physicalDevice = this->physicalDevice;
            VkDeviceSize bufferSize = sizeof(Particle) * this->objects.size();

            this->InputPartucleBuffer.device = logicaldevice;
            this->InputPartucleBuffer.size = bufferSize;
            this->InputPartucleBuffer.usageFlags = usage;
            this->InputPartucleBuffer.memoryPropertyFlags = properties;

            this->OutputPartucleBuffer.device = logicaldevice;
            this->OutputPartucleBuffer.size = bufferSize;
            this->OutputPartucleBuffer.usageFlags = usage;
            this->OutputPartucleBuffer.memoryPropertyFlags = properties;

            this->InputPartucleBuffer.createBuffer(physicalDevice);
            this->OutputPartucleBuffer.createBuffer(physicalDevice);

            PaterialBuffer staging;
            staging.device = logicaldevice;
            staging.size = bufferSize;
            staging.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            staging.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            staging.createBuffer(physicalDevice);
            staging.mapToMeBuffer(bufferSize, 0);
            staging.copyToMeBuffer(this->objects.data(), bufferSize);

            helper::copyBuffer(
                this->logicaldevice,
                this->commandPool,
                this->graphicsVKQueue,
                staging.buffer,
                this->InputPartucleBuffer.buffer,
                bufferSize);

            helper::copyBuffer(
                this->logicaldevice,
                this->commandPool,
                this->graphicsVKQueue,
                staging.buffer,
                this->OutputPartucleBuffer.buffer,
                bufferSize);

            staging.cleanup();
        }

        void GLTFmodelObject::updateUniformBuffer(cMat4 world, Camera* camera)
        {
            UniformBufferObject ubo{};
            ubo.model = cMat4(1.0f);
            ubo.view = camera->getViewMatrix();
            ubo.proj = camera->getProjectionMatrix();
            ubo.inverseTranspose = cMat4(1.0f); // skymap은 역행렬이 필요없음

            // 임시로 하나로 설정
            memcpy(this->modelviewprojUniformBuffer[0].mapped, &ubo, sizeof(ubo));
        }

        VKBaseObject::VKBaseObject(const cString& name) {}
        VKBaseObject::VKBaseObject(VKdeviceHandler* device, const cString& name)
            :
            physicalDevice(device->physicalDevice),
            logicaldevice(device->logicaldevice),
            commandPool(device->commandPool),
            graphicsVKQueue(device->graphicsVKQueue) {
        };
    }
}

