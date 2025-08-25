#include "VKengineOBJLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../common/tiny_obj_loader.h"

namespace vkengine {
    namespace helper {
        namespace loadModel {
            namespace OBJ {
                void loadAsset(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
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

                            // Tangent 정보 없음: 0으로 초기화
                            vertex.inTangent = { 0.0f, 0.0f, 0.0f, 0.0f };

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
            }
        }
    }
}