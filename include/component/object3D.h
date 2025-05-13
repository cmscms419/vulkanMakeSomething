#ifndef INCLUDE_OBJECT3D_H_
#define INCLUDE_OBJECT3D_H_ 

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKbuffer.h"
#include "VKtexture.h"

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
        private:
            VertexBuffer vertexBuffer;
            IndexBuffer indexBuffer;
        };

        class TextureArrayObject3D : public Object
        {
        public:
            void createCubeTexture(std::vector<std::string> faces);
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

    }
}



#endif // !INCLUDE_OBJECT3D_H_
