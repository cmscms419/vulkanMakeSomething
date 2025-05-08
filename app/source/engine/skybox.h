#ifndef INCLUDE_SKYBOX_H_
#define INCLUDE_SKYBOX_H_

#include "../../common/common.h"
#include "../../common/struct.h"
#include "../../common/macros.h"

#include "VKbuffer.h"
#include "VKtexture.h"

namespace vkengine {
    namespace object {
        class Skybox {
        public:
            Skybox() {};
            ~Skybox() {};
            void createCubeMap(std::vector<std::string> faces);
            void createVertexBuffer(std::vector<SkyboxVertex> vertices);
            void createIndexBuffer(std::vector<uint16_t> indices);
            void draw(VkCommandBuffer commandBuffer, uint32_t imageIndex);
            void cleanup() {
                this->vertexBuffer.cleanup();
                this->indexBuffer.cleanup();
                this->cubeMap.cleanup();
            };
            VKcubeMap getCubeMap() { return this->cubeMap; };

        private:
            SkyboxBuffer vertexBuffer;
            IndexBuffer indexBuffer;
            VKcubeMap cubeMap = {};
        };
    }
}

#endif // !INCLUDE_SKYBOX_H_