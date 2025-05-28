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
            Object() {};
            ~Object() {};
            void createVertexBuffer(std::vector<Vertex>& vertices = std::vector<Vertex>());
            void createIndexBuffer(std::vector<uint16_t>& indices = std::vector<uint16_t>());
            void createModelViewProjBuffers();
            
            virtual void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);
            virtual void cleanup() {
                this->vertexBuffer.cleanup();
                this->indexBuffer.cleanup();

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                {
                    modelviewprojUniformBuffer[i].cleanup();
                }
            };
            
            VertexBuffer* getVertexBuffer() { return &this->vertexBuffer; }
            IndexBuffer* getIndexBuffer() { return &this->indexBuffer; }
            std::string getName() { return this->name; }
            std::vector<Vertex>* getVertices() { return &this->vertexBuffer.vertices; }
            std::vector<uint16_t>* getIndices() { return &this->indexBuffer.indices; }
            glm::vec3 getPosition() { return this->position; }
            glm::quat getRotation() { return this->rotation; }
            glm::vec3 getScale() { return this->scale; }
            glm::mat4 getMatrix() { return this->matrix; }
            UniformBuffer* getModelViewProjUniformBuffer(uint16_t currentImage) { return &this->modelviewprojUniformBuffer[currentImage]; }

            void setName(std::string name) { this->name = name; }
            void setVertices(std::vector<Vertex>& vertices) { this->vertexBuffer.vertices = vertices; }
            void setIndices(std::vector<uint16_t>& indices) { this->indexBuffer.indices = indices; }
            void setPosition(glm::vec3 position) { this->position = position; }
            void setRotation(glm::quat rotation) { this->rotation = rotation; }
            void setScale(glm::vec3 scale) { this->scale = scale; }
            void setMatrix(glm::mat4 matrix) { this->matrix = matrix; }

            void CaluateRotation(glm::quat rotation)
            {
              this->rotation *= rotation;
            }

            void RotationAngle(float angle, glm::vec3 axis);
            void updateMatrix();

        protected:
            VertexBuffer vertexBuffer;
            IndexBuffer indexBuffer;
            UniformBuffer modelviewprojUniformBuffer[MAX_FRAMES_IN_FLIGHT]; // swapChain image 개수만큼 uniform buffer 생성
            std::string name = "";

            glm::vec3 position = { 0.0f, 0.0f, 0.0f };
            glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
            glm::mat4 matrix = glm::mat4(1.0f);

        };

        class Texture3DObject : public Object
        {
        public:
            void createTexture(std::string path);
            virtual void cleanup() {
                Object::cleanup();
                this->texture.cleanup();
            };
        private:
            Vk2DTexture texture = {};
        };

        class TextureArrayObject3D : public Object
        {
        public:
            void createTextureArray(std::vector<std::string> faces);
            virtual void cleanup() {
                Object::cleanup();
                this->cubeTextureArray.cleanup();
            };
            Vk2DTextureArray getCubeTexture() { return this->cubeTextureArray; }
        private:
            Vk2DTextureArray cubeTextureArray = {};
        };

        class SkyBox : public Object
        {
        public:
            void createCubeMap(std::vector<std::string> faces);
            void updateUniformBuffer(uint32_t currentImage, Camera* camera);

            virtual void cleanup() {
                Object::cleanup();

                this->cubeMap.cleanup();
            };

            VKcubeMap* getCubeMap() { return &this->cubeMap; }

        private:
            VKcubeMap cubeMap = {};
             
        };

        class ModelObject : public Object
        {
        public:
            void createTexture(std::string path);
            void updateUniformBuffer(uint32_t currentImage, glm::mat4 world, Camera* camera);

            Vk2DTexture* getTexture() { return &this->texture; }
            
            virtual void cleanup() {
                Object::cleanup();
                this->texture.cleanup();
            };

        private:
            Vk2DTexture texture = {};
        };

    }
}



#endif // !INCLUDE_OBJECT3D_H_
