#ifndef INCLUDE_VK_LOAD_MODEL_H_
#define INCLUDE_VK_LOAD_MODEL_H_


#include "common.h"
#include "macros.h"
#include "struct.h"

#include <unordered_map>

namespace vkengine {
    namespace helper {
        namespace loadModel {
            void loadModel(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
            void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
            void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);
        }
    }
}


#endif // INCLUDE_VK_LOAD_MODEL_H_