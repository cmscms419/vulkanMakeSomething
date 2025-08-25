#include "VKengineGLTFLoader.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <glm/gtc/type_ptr.hpp>

namespace vkengine {
    namespace helper {
        namespace loadModel {
            namespace GLTF {

                template<typename T>
                const T* getAttributeBuffer(const std::string& attributeName, const tinygltf::Primitive& primitive, const tinygltf::Model& model, size_t* outCount = NULL)
                {
                    auto it = primitive.attributes.find(attributeName);
                    if (it == primitive.attributes.end())
                        return nullptr;

                    const tinygltf::Accessor& accessor = model.accessors[it->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];

                    if (outCount)
                    {
                        *outCount = accessor.count;
                    }

                    return reinterpret_cast<const T*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                std::vector<cMaterial> loadMaterials(const tinygltf::Model& glTFInput)
                {
                    std::vector<cMaterial> materials;

                    if (!glTFInput.materials.empty()) {

                        for (const auto& material : glTFInput.materials) {
                            cMaterial mat{};

                            if (material.values.find("baseColorFactor") != material.values.end())
                            {
                                cVec4 baseColorFactor = glm::make_vec4(material.values.at("baseColorFactor").ColorFactor().data());
                                mat.a = baseColorFactor.a;
                                mat.r = baseColorFactor.r;
                                mat.g = baseColorFactor.g;
                                mat.b = baseColorFactor.b;

                                _PRINT_TO_CONSOLE_("Material: %s, Base Color Factor: (%.2f, %.2f, %.2f, %.2f)\n", material.name.c_str(), mat.r, mat.g, mat.b, mat.a);
                            }

                            if (material.values.find("baseColorTexture") != material.values.end()) {
#if 1
                                mat.isAlbedoMap = true;
                                mat.baseColorTextureIndex = material.values.at("baseColorTexture").TextureIndex();
                                _PRINT_TO_CONSOLE_("Albedo map found for material: %s\n", material.name.c_str());
#else
                                _PRINT_TO_CONSOLE_("but not implemented yet.\n");
                                mat.isAlbedoMap = false;
#endif
                            }
                            materials.push_back(mat);
                        }
                    }
                    else {
                        _PRINT_TO_CONSOLE_("No materials found in the glTF model.\n");
                    }

                    return materials;
                }

                std::vector<cUint32_t> loadTextures(const tinygltf::Model& glTFInput)
                {
                    std::vector<cUint32_t> indexes;

                    if (!glTFInput.textures.empty()) {
                        for (const auto& texture : glTFInput.textures) {
                            cUint32_t textureIndex = texture.source;
                            indexes.push_back(textureIndex);

                            _PRINT_TO_CONSOLE_("Texture found: %s, Index: %u\n", texture.name.c_str(), textureIndex);

                        }
                    }
                    else {
                        _PRINT_TO_CONSOLE_("No textures found in the glTF model.\n");
                    }

                    return indexes;
                }

                std::vector<tinygltf::Scene> loadScene(const tinygltf::Model& glTFInput)
                {
                    std::vector<tinygltf::Scene> scenes;

                    if (!glTFInput.scenes.empty()) {
                        for (const auto& scene : glTFInput.scenes) {
                            scenes.push_back(scene);
                            _PRINT_TO_CONSOLE_("Scene found: %s, Index: %zu\n", scene.name.c_str(), &scene - &glTFInput.scenes[0]);
                        }
                    }
                    else {
                        _PRINT_TO_CONSOLE_("No scenes found in the glTF model.\n");
                    }
                    return scenes;

                }

                Node* loadNodes(const tinygltf::Node& NodeInput, const tinygltf::Model& glTFInput, Node* parent, std::vector<uint32_t>& indeies, std::vector<Vertex>& vertices)
                {
                    _PRINT_TO_CONSOLE_("Loading node: %s\n", NodeInput.name.c_str());
                    Node* node = new Node();
                    node->matrix = cMat4(1.0f); // 기본 단위 행렬로 초기화
                    node->parent = parent;

                    if (NodeInput.translation.size() == 3) {
                        node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(NodeInput.translation.data())));
                        _PRINT_TO_CONSOLE_("Node translation: (%.2f, %.2f, %.2f)\n", NodeInput.translation[0], NodeInput.translation[1], NodeInput.translation[2]);
                    }

                    if (NodeInput.rotation.size() == 4) {
                        glm::quat q = glm::make_quat(NodeInput.rotation.data());
                        node->matrix *= glm::mat4(q);
                        _PRINT_TO_CONSOLE_("Node rotation: (%.2f, %.2f, %.2f, %.2f)\n", NodeInput.rotation[0], NodeInput.rotation[1], NodeInput.rotation[2], NodeInput.rotation[3]);
                    }

                    if (NodeInput.scale.size() == 3) {
                        node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(NodeInput.scale.data())));
                        _PRINT_TO_CONSOLE_("Node scale: (%.2f, %.2f, %.2f)\n", NodeInput.scale[0], NodeInput.scale[1], NodeInput.scale[2]);
                    }

                    if (NodeInput.matrix.size() == 16) {
                        node->matrix = glm::make_mat4x4(NodeInput.matrix.data());
                        _PRINT_TO_CONSOLE_("Node matrix: \n");
                    };

                    if (NodeInput.children.size() > 0)
                    {
                        for (auto& child : NodeInput.children)
                        {
                            _PRINT_TO_CONSOLE_("Node %s has child: %s\n", NodeInput.name.c_str(), glTFInput.nodes[child].name.c_str());
                            loadNodes(glTFInput.nodes[child], glTFInput, node, indeies, vertices);
                        }
                    }

                    if (NodeInput.mesh > -1)
                    {
                        const tinygltf::Mesh mesh = glTFInput.meshes[NodeInput.mesh];

                        for (size_t i = 0; i < mesh.primitives.size(); i++) {

                            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];

                            uint32_t firstIndex = static_cast<uint32_t>(indeies.size());
                            uint32_t vertexStart = static_cast<uint32_t>(vertices.size());
                            uint32_t indexCount = 0;

                            // vertices Data
                            {
                                const float* positionBuffer = nullptr;
                                const float* normalsBuffer = nullptr;
                                const float* texCoordsBuffer = nullptr;
                                const float* tangentsBuffer = nullptr;
                                const float* colorsBuffer = nullptr;
                                size_t vertexCount = 0;
#if 0
                                // Get buffer data for vertex positions
                                if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                                    const tinygltf::Accessor& accessor = glTFInput.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                                    const tinygltf::BufferView& view = glTFInput.bufferViews[accessor.bufferView];
                                    positionBuffer = reinterpret_cast<const float*>(&(glTFInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                                    vertexCount = accessor.count;
                                }
                                // Get buffer data for vertex normals
                                if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                                    const tinygltf::Accessor& accessor = glTFInput.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                                    const tinygltf::BufferView& view = glTFInput.bufferViews[accessor.bufferView];
                                    normalsBuffer = reinterpret_cast<const float*>(&(glTFInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                                }
                                // Get buffer data for vertex texture coordinates
                                // glTF supports multiple sets, we only load the first one
                                if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                                    const tinygltf::Accessor& accessor = glTFInput.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                                    const tinygltf::BufferView& view = glTFInput.bufferViews[accessor.bufferView];
                                    texCoordsBuffer = reinterpret_cast<const float*>(&(glTFInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                                }

                                if (glTFPrimitive.attributes.find("TANGENT") != glTFPrimitive.attributes.end()) {
                                    const tinygltf::Accessor& accessor = glTFInput.accessors[glTFPrimitive.attributes.at("TANGENT")];
                                    const tinygltf::BufferView& view = glTFInput.bufferViews[accessor.bufferView];
                                    tangentsBuffer = reinterpret_cast<const float*>(&(glTFInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                                }
#else
                                positionBuffer = getAttributeBuffer<float>("POSITION", glTFPrimitive, glTFInput, &vertexCount);
                                normalsBuffer = getAttributeBuffer<float>("NORMAL", glTFPrimitive, glTFInput);
                                texCoordsBuffer = getAttributeBuffer<float>("TEXCOORD_0", glTFPrimitive, glTFInput);
                                tangentsBuffer = getAttributeBuffer<float>("TANGENT", glTFPrimitive, glTFInput);
                                colorsBuffer = getAttributeBuffer<float>("COLOR_0", glTFPrimitive, glTFInput);
#endif

                                // Append data to model's vertex buffer
                                for (size_t v = 0; v < vertexCount; v++) {
                                    Vertex vert{};

                                    vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                                    vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                                    vert.texCoord = texCoordsBuffer ? cVec3(glm::make_vec2(&texCoordsBuffer[v * 2]), 0.0f) : glm::vec3(0.0f);
#if 0
                                    vert.inTangent = tangentsBuffer ? glm::vec4(glm::make_vec4(&tangentsBuffer[v * 4])) : glm::vec4(1.0f); // Tangent가 아닌 shader에서 color로 사용될 수 있음
#else
                                    vert.inTangent = glm::vec4(1.0f); // Tangent가 아닌 shader에서 color로 사용될 수 있음
#endif
                                    /*_PRINT_TO_CONSOLE_(
                                        "Vertex %zu: Position: (%.2f, %.2f, %.2f), Normal: (%.2f, %.2f, %.2f), TexCoord: (%.2f, %.2f)\n",
                                        v, vert.pos.x, vert.pos.y, vert.pos.z,
                                        vert.normal.x, vert.normal.y, vert.normal.z,
                                        vert.texCoord.x, vert.texCoord.y);*/

                                    vertices.push_back(vert);
                                }
                            }

                            // indeies Data
                            {
                                const tinygltf::Accessor& indexAccessor = glTFInput.accessors[glTFPrimitive.indices];
                                const tinygltf::BufferView& indexView = glTFInput.bufferViews[indexAccessor.bufferView];
                                const tinygltf::Buffer& indexBuffer = glTFInput.buffers[indexView.buffer];

                                indexCount += static_cast<uint32_t>(indexAccessor.count);

                                _PRINT_TO_CONSOLE_("Primitive %zu: Index Count: %d\n", i, indexCount);
                                _PRINT_TO_CONSOLE_("Primitive %zu: First Index: %u, Vertex Start: %u\n", i, firstIndex, vertexStart);

                                switch (indexAccessor.componentType) {
                                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                                    const cUint32_t* indicesBuffer = reinterpret_cast<const cUint32_t*>(&indexBuffer.data[indexAccessor.byteOffset + indexView.byteOffset]);
                                    for (size_t j = 0; j < indexAccessor.count; j++) {
                                        indeies.push_back(indicesBuffer[j] + vertexStart);
                                    }
                                } break;
                                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                                    const cUint16_t* indicesBuffer = reinterpret_cast<const cUint16_t*>(&indexBuffer.data[indexAccessor.byteOffset + indexView.byteOffset]);
                                    for (size_t j = 0; j < indexAccessor.count; j++) {
                                        indeies.push_back(indicesBuffer[j] + vertexStart);
                                    }
                                } break;
                                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                                    const cUInt8_t* indicesBuffer = reinterpret_cast<const cUInt8_t*>(&indexBuffer.data[indexAccessor.byteOffset + indexView.byteOffset]);
                                    for (size_t j = 0; j < indexAccessor.count; j++) {
                                        indeies.push_back(indicesBuffer[j] + vertexStart);
                                    }
                                } break;
                                default:
                                    _PRINT_TO_CONSOLE_("Unsupported index type in glTF primitive.\n");
                                    return nullptr;
                                }
                            }

                            // Primitive Data Update
                            Primitive primitive;
                            primitive.firstIndex = firstIndex;
                            primitive.indexCount = indexCount;
                            primitive.materialIndex = glTFPrimitive.material;
                            node->mesh.primitives.push_back(primitive);
                        }

                    }

                    if (parent) {
                        parent->children.push_back(node);

                        Node* empty = new Node();
                        return empty;
                    }
                    else
                    {
                        return node;
                    }

                }

                void loadAsset(const std::string& path, vkengine::object::GLTFmodelObject* modelObject)
                {
                    tinygltf::Model glTFInput;
                    tinygltf::TinyGLTF gltfContext;

                    cString error, warning;

                    cString root = vkengine::VulkanEngine::Get().getRootPath();
                    cString resoursePath = vkengine::VulkanEngine::Get().getResourcePath() + path;
                    cString imgResourcePath;
                    cString fullpath = root + resoursePath;

                    bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, fullpath);

                    if (!fileLoaded)
                    {
                        _PRINT_TO_CONSOLE_("Failed to load glTF file: %s\n", error.c_str());
                        return;
                    }

                    _PRINT_TO_CONSOLE_("Loaded glTF file: %s\n", path.c_str());

                    std::vector<cUint32_t> indices;
                    std::vector<Vertex> vertices;

                    std::vector<tinygltf::Scene> scenes;
                    std::vector<Node*> nodes;
                    std::vector<tinygltf::Image> images;
                    std::vector<cMaterial> materials;
                    std::vector<cUint32_t> textures;

                    images = glTFInput.images;
                    materials = GLTF::loadMaterials(glTFInput);
                    textures = GLTF::loadTextures(glTFInput);
                    scenes = GLTF::loadScene(glTFInput);

                    tinygltf::Scene scene = glTFInput.scenes[0];

                    for (size_t i = 0; i < scene.nodes.size(); i++)
                    {
                        const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
                        nodes.push_back(GLTF::loadNodes(node, glTFInput, nullptr, indices, vertices));
                    }
                    
                    // ModelObject에 데이터 설정
                    modelObject->getVertices()->assign(vertices.begin(), vertices.end());
                    modelObject->getIndices()->assign(indices.begin(), indices.end());

                    modelObject->textureIndexes = textures;
                    modelObject->materials = materials;
                    modelObject->nodes = nodes;

                    size_t extPos = resoursePath.find(".gltf");
                    if (extPos != std::string::npos) {
                        // ".gltf"가 포함된 파일명 앞의 '/' 위치 찾기
                        size_t lastSlash = resoursePath.rfind('/', extPos);
                        if (lastSlash != std::string::npos) {
                            resoursePath.erase(lastSlash + 1);
                        }
                    }
                    imgResourcePath = root + resoursePath;

                    std::vector<Vk2DTexture*> texturesArray;

                    for (auto& image : images)
                    {
                        TextureResourceBase* resource{};

                        _PRINT_TO_CONSOLE_("Image found: %s, Width: %d, Height: %d\n", image.uri.c_str(), image.width, image.height);

                        if (image.width == 0 || image.height == 0) {
                            _PRINT_TO_CONSOLE_("Image %s has invalid dimensions (%d x %d), skipping.\n", image.uri.c_str(), image.width, image.height);
                            continue;
                        }

                        // ".png" 문자열이 포함되어 있는지 확인
                        if (image.uri.find(".png") != std::string::npos) {
                            _PRINT_TO_CONSOLE_("Image %s contains '.png' extension.\n", image.uri.c_str());

                            resource = new TextureResourcePNG();
                            resource->texWidth = image.width;
                            resource->texHeight = image.height;
                        }
                        else if (image.uri.find(".ktx") != std::string::npos)
                        {
                            _PRINT_TO_CONSOLE_("Image %s contains '.ktx' extension.\n", image.uri.c_str());

                            resource = new TextureResourceKTX();
                            resource->texWidth = image.width;
                            resource->texHeight = image.height;
                        }
                        else {
                            _PRINT_TO_CONSOLE_("Image %s does not contain extension.\n", image.uri.c_str());
                            continue;
                        }

                        cChar* imageName = const_cast<cChar*>(image.uri.c_str());

                        resource->createResource(imgResourcePath + image.uri, imageName);

                        switch (resource->type)
                        {
                        case TextureResourceBase::TYPE::PNG:
                            modelObject->setTexturePNG(static_cast<TextureResourcePNG*>(resource));
                            break;
                        case TextureResourceBase::TYPE::KTX:
                            modelObject->setTextureKTX(static_cast<TextureResourceKTX*>(resource));
                            break;
                        default:
                            break;
                        }
                        
                    }

                    modelObject->createTexture();

                }


            } // namespace GLTF
        } // namespace loadModel
    } // namespace helper
} // namespace vkengine
