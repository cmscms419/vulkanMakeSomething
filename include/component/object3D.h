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

namespace vkengine {
    namespace object {

        class Object {
        public:
            Object() {};
            ~Object() {};
            void createVertexBuffer(std::vector<Vertex> vertices);
            void createIndexBuffer(std::vector<uint16_t> indices);
            
            virtual void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);
            virtual void cleanup() {
                this->vertexBuffer.cleanup();
                this->indexBuffer.cleanup();
            };
            
            VertexBuffer* getVertexBuffer() { return &this->vertexBuffer; }
            IndexBuffer* getIndexBuffer() { return &this->indexBuffer; }
            
            void setPosition(glm::vec3 position) { this->position = position; }
            void setRotation(glm::quat rotation) { this->rotation = rotation; }
            void setScale(glm::vec3 scale) { this->scale = scale; }
            void setMatrix(glm::mat4 matrix) { this->matrix = matrix; }
            void RotationAngle(float angle, glm::vec3 axis);
            void updateMatrix();

            glm::vec3 getPosition() { return this->position; }
            glm::quat getRotation() { return this->rotation; }
            glm::vec3 getScale() { return this->scale; }
            glm::mat4 getMatrix() { return this->matrix; }

        private:

            VertexBuffer vertexBuffer;
            IndexBuffer indexBuffer;

            glm::vec3 position = { 0.0f, 0.0f, 0.0f };
            glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
            glm::mat4 matrix = glm::mat4(1.0f);

        };

        class TextureObject3D : public Object
        {
        public:
            void createTexture(std::string path);
            std::vector<Vertex>* getVertices() { return &this->vertices; }
            std::vector<uint16_t>* getIndices() { return &this->indices; }

            virtual void cleanup() {
                Object::cleanup();
                this->texture.cleanup();
            };
        private:
            Vk2DTexture texture = {};
            // 임시 버퍼
            std::vector<Vertex> vertices;
            std::vector<uint16_t> indices;
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
            void createCubeMap(std::vector<std::string> faces, VKDevice_& device);
            virtual void cleanup() {
                Object::cleanup();

                this->cubeMap.cleanup();
            };
            VKcubeMap getCubeMap() { return this->cubeMap; }
        private:
            VKcubeMap cubeMap = {};
        };

        class ModelObject : public Object
        {
        public:
            void createTexture(std::string path);
            std::vector<Vertex>* getVertices() { return &this->vertices; }
            std::vector<uint16_t>* getIndices() { return &this->indices; }

            virtual void cleanup() {
                Object::cleanup();
                this->texture.cleanup();
            };
            Vk2DTexture getTexture() { return this->texture; }
            
        private:
            Vk2DTexture texture = {};
            // 임시 버퍼
            std::vector<Vertex> vertices;
            std::vector<uint16_t> indices;
        };

    }
}



#endif // !INCLUDE_OBJECT3D_H_
