
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "VKLoadModel2.h"

using namespace tinygltf;

namespace vkengine {
    namespace helper {
        namespace vkglTF
        {
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
#if 0
                        // 인덱스
                        std::vector<uint16_t> localIndices;
                        if (primitive.indices >= 0) {
                            const Accessor& idxAccessor = model.accessors[primitive.indices];
                            const BufferView& idxView = model.bufferViews[idxAccessor.bufferView];
                            const Buffer& idxBuffer = model.buffers[idxView.buffer];

                            const void* dataPtr = &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset];

                            for (size_t i = 0; i < idxAccessor.count; ++i) {
                                uint16_t idx = 0;
                                if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                                    idx = reinterpret_cast<const uint16_t*>(dataPtr)[i];
                                }
                                else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                                    idx = static_cast<uint16_t>(reinterpret_cast<const uint32_t*>(dataPtr)[i]);
                                }
                                else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                                    idx = reinterpret_cast<const uint8_t*>(dataPtr)[i];
                                }
                                localIndices.push_back(idx);
                            }
                        }
#else
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
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                            cUint16_t* buf = new cUint16_t[idxAccessor.count];
                            memcpy(buf, &idxBuffer.data[idxAccessor.byteOffset + idxView.byteOffset], idxAccessor.count * sizeof(cUint16_t));
                            for (size_t index = 0; index < idxAccessor.count; index++) {
                                indices.push_back(buf[index] + static_cast<cUint16_t>(vertices.size()));
                            }
                            delete[] buf;
                            break;
                        }
                        default:
                            return;
                        }

#endif
                        size_t vertexCount = posAccessor.count;
                        for (size_t i = 0; i < vertexCount; ++i) {
                            Vertex v;
                            v.pos = cVec3(
                                positions[i * 3 + 0],
                                positions[i * 3 + 1],
                                positions[i * 3 + 2]
                            );
                            if (normals)
                                v.normal = cVec3(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
                            else
                                v.normal = cVec3(0.0f, 0.0f, 0.0f);

                            if (texcoords)
                                v.texCoord = cVec3(texcoords[i * 2 + 0], texcoords[i * 2 + 1], 0.0f);
                            else
                                v.texCoord = cVec3(0.0f, 0.0f, 0.0f);

                            vertices.push_back(v);
                        }
                        
                        continue;
                        // 인덱스 추가
                        //if (!localIndices.empty()) {
                        //    for (auto idx : localIndices) {
                        //        indices.push_back(idx);
                        //    }
                        //}
                        //else {
                        //    // 인덱스가 없으면 0,1,2,... 순서로
                        //    for (uint16_t i = 0; i < vertexCount; ++i) {
                        //        indices.push_back(i);
                        //    }
                        //}
                    }
                }
            }
        }
    }
}

