#include "VKLoadModel.h"

// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../common/tiny_obj_loader.h"

using namespace tinygltf;

namespace vkengine {
    namespace helper {
        namespace loadModel {
            void loadModel(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
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

            void loadModelGLTF(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices)
            {
                Model model;
                TinyGLTF loader;
                std::string err, warn;

                bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
                if (!ret) {
                    printf("Failed to load glTF: %s\n", err.c_str());
                    return;
                }

                // 아직 1개의 Node만 지원
                for (const auto& mesh : model.meshes) {
                    for (const auto& primitive : mesh.primitives) {
                        // Attribute accessor 인덱스 얻기
                        const auto& attributes = primitive.attributes;

                        // POSITION
                        const Accessor& posAccessor = model.accessors[attributes.at("POSITION")];
                        const BufferView& posView = model.bufferViews[posAccessor.bufferView];
                        const Buffer& posBuffer = model.buffers[posView.buffer];
                        const float* positions = reinterpret_cast<const float*>(
                            &posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

                        // NORMAL
                        const float* normals = nullptr;
                        if (attributes.find("NORMAL") != attributes.end()) {
                            const Accessor& normalAccessor = model.accessors[attributes.at("NORMAL")];
                            const BufferView& normalView = model.bufferViews[normalAccessor.bufferView];
                            const Buffer& normalBuffer = model.buffers[normalView.buffer];
                            normals = reinterpret_cast<const float*>(
                                &normalBuffer.data[normalView.byteOffset + normalAccessor.byteOffset]);
                        }

                        // TEXCOORD_0
                        const float* texcoords = nullptr;
                        if (attributes.find("TEXCOORD_0") != attributes.end()) {
                            const Accessor& texAccessor = model.accessors[attributes.at("TEXCOORD_0")];
                            const BufferView& texView = model.bufferViews[texAccessor.bufferView];
                            const Buffer& texBuffer = model.buffers[texView.buffer];
                            texcoords = reinterpret_cast<const float*>(
                                &texBuffer.data[texView.byteOffset + texAccessor.byteOffset]);
                        }

                        // TANGENT
                        const float* tangents = nullptr;
                        if (attributes.find("TANGENT") != attributes.end()) {
                            const Accessor& tanAccessor = model.accessors[attributes.at("TANGENT")];
                            const BufferView& tanView = model.bufferViews[tanAccessor.bufferView];
                            const Buffer& tanBuffer = model.buffers[tanView.buffer];
                            tangents = reinterpret_cast<const float*>(
                                &tanBuffer.data[tanView.byteOffset + tanAccessor.byteOffset]);
                        }


                        const Accessor& idxAccessor = model.accessors[primitive.indices];
                        const BufferView& idxView = model.bufferViews[idxAccessor.bufferView];
                        const Buffer& idxBuffer = model.buffers[idxView.buffer];

                        cUint16_t indexCount = static_cast<cUint16_t>(idxAccessor.count);

                        switch (idxAccessor.componentType) {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                            cUint32_t* buf = new cUint32_t[idxAccessor.count];
                            memcpy(buf, &idxBuffer.data[idxAccessor.byteOffset + idxView.byteOffset], idxAccessor.count * sizeof(cUint32_t));
                            for (size_t index = 0; index < idxAccessor.count; index++) {
                                indices.push_back(buf[index] + static_cast<cUint32_t>(vertices.size()));
                            }
                            delete[] buf;
                        }
                        break;
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                            cUint16_t* buf = new cUint16_t[idxAccessor.count];
                            memcpy(buf, &idxBuffer.data[idxAccessor.byteOffset + idxView.byteOffset], idxAccessor.count * sizeof(cUint16_t));
                            for (size_t index = 0; index < idxAccessor.count; index++) {
                                indices.push_back(buf[index] + static_cast<cUint16_t>(vertices.size()));
                            }
                            delete[] buf;
                        }
                        break;
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                            cUInt8_t* buf = new cUInt8_t[idxAccessor.count];
                            memcpy(buf, &idxBuffer.data[idxAccessor.byteOffset + idxView.byteOffset], idxAccessor.count * sizeof(cUInt8_t));
                            for (size_t index = 0; index < idxAccessor.count; index++) {
                                indices.push_back(buf[index] + static_cast<cUInt8_t>(vertices.size()));
                            }
                            delete[] buf;
                        }
                        break;

                        default:
                            return;
                        }

                        size_t vertexCount = posAccessor.count;

                        for (size_t i = 0; i < vertexCount; ++i) {
                            Vertex v;
                            v.pos = cVec3(positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2]
                            );
                            if (normals)
                            {
                                v.normal = cVec3(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
                            }
                            else
                            {
                                v.normal = cVec3(0.0f, 0.0f, 0.0f);
                            }

                            if (texcoords)
                            {
                                v.texCoord = cVec3(texcoords[i * 2 + 0], texcoords[i * 2 + 1], 0.0f);
                            }
                            else
                            {
                                v.texCoord = cVec3(0.0f, 0.0f, 0.0f);
                            }

                            if (tangents)
                            {
                                v.inTangent = cVec4(tangents[i * 4 + 0], tangents[i * 4 + 1], tangents[i * 4 + 2], tangents[i * 4 + 3]);
                            }
                            else
                            {
                                v.inTangent = cVec4(0.0f, 0.0f, 0.0f, 0.0f);
                            }

                            vertices.push_back(v);
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