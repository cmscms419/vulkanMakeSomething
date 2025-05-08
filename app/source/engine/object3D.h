#ifndef INCLUDE_OBJECT3D_H_
#define INCLUDE_OBJECT3D_H_ 

#include "../../common/common.h"
#include "../../common/struct.h"
#include "../../common/macros.h"

#include "VKbuffer.h"

namespace vkengine {
    namespace object {
        class Object3D {
        public:
            Object3D() {};
            ~Object3D() {};
            void createVertexBuffer(std::vector<Vertex> vertices);
            void createIndexBuffer(std::vector<uint16_t> indices);
            void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);
            void cleanup() {
                this->vertexBuffer.cleanup();
                this->indexBuffer.cleanup();
            };
        private:
            VertexBuffer vertexBuffer;
            IndexBuffer indexBuffer;
        };
    }
}

#endif // !INCLUDE_OBJECT3D_H_
