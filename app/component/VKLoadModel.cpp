#include "VKLoadModel.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../common/tiny_obj_loader.h"

namespace vkengine {
    namespace helper {
        namespace loadModel {

            void loadModel(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
            {
                tinyobj::attrib_t attrib;
                std::vector<tinyobj::shape_t> shapes;
                std::vector<tinyobj::material_t> materials;
                std::string warn, err;

                std::string obj = path + RESOURSE_PATH + MODEL_PATH;

                _CHECK_RESULT_(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj.c_str()));

                std::vector<Vertex> uniqueVertices;

                for (const auto& shape : shapes) {
                    for (const auto& index : shape.mesh.indices) {
                        Vertex vertex{};
                        vertex.pos = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                        };

                        vertex.color = { 1.0f, 1.0f, 1.0f };

                        vertex.texCoord = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                            0.0f // z°ªÀ» 0.0f·Î
                        };

#if UNIQUE_VERTEXTYPE
                        auto it = std::find(vertices.begin(), vertices.end(), vertex);
                        if (it == vertices.end()) {
                            vertices.push_back(vertex);
                            indices.push_back(static_cast<uint16_t>(vertices.size() - 1));
                        }
                        else {
                            indices.push_back(static_cast<uint16_t>(std::distance(vertices.begin(), it)));
                        }
#else
                        /*this->VKvertices.push_back(vertex);
                        this->VKindices.push_back(this->VKindices.size());*/
#endif
                    }
                }

            }

            void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory)
            {

            }

            void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory)
            {

            }
        }
    }
}