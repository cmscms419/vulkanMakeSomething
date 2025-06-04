#ifndef INCLUDE_OBJECT3D_H_
#define INCLUDE_OBJECT3D_H_ 

#include "common.h"
#include "struct.h"
#include "macros.h"
#include "vkmath.h"

#include "VKengine.h"
#include "VKbuffer.h"
#include "VKtexture.h"
#include "helper.h"
#include "VKDescriptor.h"
#include "Camera.h"

namespace vkengine {
    namespace object {

        class Object {
        public:
            Object(VKDevice_* device) : device(device) {};
            ~Object() {};
            void createVertexBuffer(std::vector<Vertex>& vertices = std::vector<Vertex>());
            void createIndexBuffer(std::vector<cUint16_t>& indices = std::vector<cUint16_t>());
            void createModelViewProjBuffers();
            void createTexture(VkFormat format = VK_FORMAT_R8G8B8A8_SRGB)
            {
                this->texture->createTextureImage();
                this->texture->createTextureImageView(format);
                this->texture->createTextureSampler();
            };

            virtual void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);
            virtual void cleanup() {
                this->vertexBuffer.cleanup();
                this->indexBuffer.cleanup();
                if (this->texture) {
                    this->texture->cleanup();
                    if (this->texture->getResource()) {
                        delete this->texture->getResource(); // Replace direct destructor call with delete  
                    }
                }

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                {
                    modelviewprojUniformBuffer[i].cleanup();
                }
            };
            
            VertexBuffer* getVertexBuffer() { return &this->vertexBuffer; }
            IndexBuffer* getIndexBuffer() { return &this->indexBuffer; }
            UniformBuffer* getModelViewProjUniformBuffer(cUint16_t currentImage) { return &this->modelviewprojUniformBuffer[currentImage]; }
            cString getName() { return this->name; }
            VkTextureBase* getTexture() { return this->texture; } // 텍스처 객체 반환
            std::vector<Vertex>* getVertices() { return &this->vertexBuffer.vertices; }
            std::vector<cUint16_t>* getIndices() { return &this->indexBuffer.indices; }
            cVec3 getPosition() { return this->position; }
            cQuat getRotation() { return this->rotation; }
            cVec3 getScale() { return this->scale; }
            cMat4 getMatrix() { return this->matrix; }

            void setName(cString name) { this->name = name; }
            void setVertices(std::vector<Vertex>& vertices) { this->vertexBuffer.vertices = vertices; }
            void setIndices(std::vector<cUint16_t>& indices) { this->indexBuffer.indices = indices; }
            void setPosition(cVec3 position) { this->position = position; }
            void setRotation(cQuat rotation) { this->rotation = rotation; }
            void setScale(cVec3 scale) { this->scale = scale; }
            void setMatrix(cMat4 matrix) { this->matrix = matrix; }
            void setTexture(TextureResource* resource) { this->texture->setResource(resource); }

            void CaluateRotation(cQuat rotation)
            {
              this->rotation *= rotation;
            }
            void RotationAngle(float angle, cVec3 axis);
            void updateMatrix();

        protected:

            VertexBuffer vertexBuffer;
            IndexBuffer indexBuffer;
            UniformBuffer modelviewprojUniformBuffer[MAX_FRAMES_IN_FLIGHT]; // swapChain image 개수만큼 uniform buffer 생성
            VkTextureBase* texture = nullptr; // 텍스처 객체
            VKDevice_* device = nullptr; // VKDevice 포인터
            cString name = "";

            cVec3 position = { 0.0f, 0.0f, 0.0f };
            cQuat rotation = cQuat(1.0f, 0.0f, 0.0f, 0.0f);
            cVec3 scale = { 1.0f, 1.0f, 1.0f };
            cMat4 matrix = cMat4(1.0f);

        };

        class TextureArrayObject3D : public Object
        {
        public:
            TextureArrayObject3D(VKDevice_* device) : Object(device) {
                this->texture = new Vk2DTextureArray();
                this->texture->setDevice(this->device);
            };
            virtual void cleanup() {
                Object::cleanup();
            };
        private:
        };

        class SkyBox : public Object
        {
        public:

            SkyBox(VKDevice_* device) : Object(device) {
                this->texture = new VKcubeMap();
                this->texture->setDevice(this->device);
            };

            void updateUniformBuffer(uint32_t currentImage, Camera* camera);

            virtual void cleanup() {
                Object::cleanup();
            };
        private:
        };

        class ModelObject : public Object
        {
        public:
            ModelObject(VKDevice_* device) : Object(device) {
                this->texture = new Vk2DTexture();
                this->texture->setDevice(this->device);
            };

            void updateUniformBuffer(uint32_t currentImage, cMat4 world, Camera* camera);

            virtual void cleanup() {
                Object::cleanup();
            };

        private:
        };

    }
}



#endif // !INCLUDE_OBJECT3D_H_
