#include "VKLoadModel.h"

// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

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

                        vertex.normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2]
                        };

                        vertex.texCoord = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            attrib.texcoords[2 * index.texcoord_index + 1],
                            0.0f // z값을 0.0f로
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

            void loadModel2(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
            {
                tinyobj::attrib_t attrib;
                std::vector<tinyobj::shape_t> shapes;
                std::vector<tinyobj::material_t> materials;
                std::string warn, err;

                std::string obj = path;

                _CHECK_RESULT_(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj.c_str()));

                std::vector<Vertex> uniqueVertices;

                for (const auto& shape : shapes) {
                    for (const auto& index : shape.mesh.indices) {
                        Vertex vertex{};
                        
                        if (!attrib.vertices.empty())
                        {
                            vertex.pos = {
                                attrib.vertices[3 * index.vertex_index + 0],
                                attrib.vertices[3 * index.vertex_index + 1],
                                attrib.vertices[3 * index.vertex_index + 2]
                            };
                        }
                        else
                        {
                            vertex.pos = { 0.0f, 0.0f, 0.0f };
                        }

                        if (!attrib.normals.empty())
                        {
                            vertex.normal = {
                                attrib.normals[3 * index.normal_index + 0],
                                attrib.normals[3 * index.normal_index + 1],
                                attrib.normals[3 * index.normal_index + 2]
                            };
                        }
                        else
                        {
                            vertex.normal = { 0.0f, 0.0f, 0.0f }; // 기본값 설정
                        }

                        if (!attrib.texcoords.empty())
                        {
                            vertex.texCoord = {
                                attrib.texcoords[2 * index.texcoord_index + 0],
                                attrib.texcoords[2 * index.texcoord_index + 1],
                                0.0f // z값을 0.0f로
                            };
                        }
                        else
                        {
                            vertex.texCoord = { 0.0f, 0.0f, 0.0f };
                        }

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

            void loadModel3(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
            {
                tinyobj::attrib_t attrib;
                std::vector<tinyobj::shape_t> shapes;
                std::vector<tinyobj::material_t> materials;
                std::string warn, err;

                std::string obj = path;

                _CHECK_RESULT_(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj.c_str()));

                std::vector<Vertex> uniqueVertices;

                for (const auto& shape : shapes) {
                    size_t indexCount = shape.mesh.indices.size();
                    for (size_t i = 0; i < indexCount; i += 3) {
                        // 3개의 vertex를 읽음
                        Vertex v[3];
                        for (int j = 0; j < 3; ++j) {
                            const auto& idx = shape.mesh.indices[i + j];
                            v[j].pos = {
                                attrib.vertices[3 * idx.vertex_index + 0],
                                attrib.vertices[3 * idx.vertex_index + 1],
                                attrib.vertices[3 * idx.vertex_index + 2]
                            };
                            if (!attrib.texcoords.empty() && idx.texcoord_index >= 0) {
                                v[j].texCoord = {
                                    attrib.texcoords[2 * idx.texcoord_index + 0],
                                    attrib.texcoords[2 * idx.texcoord_index + 1],
                                    0.0f
                                };
                            }
                            else {
                                v[j].texCoord = { 0.0f, 0.0f, 0.0f };
                            }
                        }

                        // normal 계산
                        cVec3 edge1 = cVec3(v[1].pos) - cVec3(v[0].pos);
                        cVec3 edge2 = cVec3(v[2].pos) - cVec3(v[0].pos);
                        cVec3 normal = glm::normalize(glm::cross(edge1, edge2));

                        for (int j = 0; j < 3; ++j) {
                            if (attrib.normals.empty()) {
                                // normals가 없으면 기본값으로 설정
                                for (int j = 0; j < 3; ++j) {
                                    v[j].normal = { normal.x, normal.y, normal.z };
                                }
                            }
                            else {
                                // normals가 있으면 해당 값으로 설정
                                for (int j = 0; j < 3; ++j) {
                                    v[j].normal = {
                                        attrib.normals[3 * shape.mesh.indices[i + j].normal_index + 0],
                                        attrib.normals[3 * shape.mesh.indices[i + j].normal_index + 1],
                                        attrib.normals[3 * shape.mesh.indices[i + j].normal_index + 2]
                                    };
                                }
                            }
                            // 중복 체크 후 push
#if UNIQUE_VERTEXTYPE
                            auto it = std::find(vertices.begin(), vertices.end(), v[j]);
                            if (it == vertices.end()) {
                                vertices.push_back(v[j]);
                                indices.push_back(static_cast<uint16_t>(vertices.size() - 1));
                            }
                            else {
                                indices.push_back(static_cast<uint16_t>(std::distance(vertices.begin(), it)));
                            }
#else
                            vertices.push_back(v[j]);
                            indices.push_back(static_cast<uint16_t>(vertices.size() - 1));
#endif
                        }
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