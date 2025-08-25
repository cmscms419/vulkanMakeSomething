#ifndef INLCUDE_VK_ENGINE_GLTF_LOADER_H_
#define INLCUDE_VK_ENGINE_GLTF_LOADER_H_

#include "common.h"
#include "macros.h"
#include "struct.h"

#include "object3D.h"

#include <unordered_map>

/*
 * Note that this isn't a complete glTF loader and not all features of the glTF 2.0 spec are supported
 * For details on how glTF 2.0 works, see the official spec at https://github.com/KhronosGroup/glTF/tree/master/specification/2.0
 *
 * If you are looking for a complete glTF implementation, check out https://github.com/SaschaWillems/Vulkan-glTF-PBR/
 */

namespace tinygltf {
    class Model;
    class TinyGLTF;
    class Node;
    
    struct Scene;
    struct Primitive;
}

namespace vkengine {
    namespace helper {
        namespace loadModel {
            namespace GLTF {
                
                template<typename T>
                const T* getAttributeBuffer(const std::string& attributeName, const tinygltf::Primitive& primitive, const tinygltf::Model& model, size_t& outCount);

                std::vector<cMaterial> loadMaterials(const tinygltf::Model& glTFInput);
                std::vector<cUint32_t> loadTextures(const tinygltf::Model& glTFInput);
                std::vector<tinygltf::Scene> loadScene(const tinygltf::Model& glTFInput);
                Node* loadNodes(const tinygltf::Node& NodeInput, const tinygltf::Model& glTFInput, Node* parent, std::vector<uint32_t>& indeies, std::vector<Vertex>& vertices);

                void loadAsset(const std::string& path, vkengine::object::GLTFmodelObject* modelObject);

            } // namespace GLTF
        }
    }
}



#endif // INLCUDE_VK_ENGINE_GLTF_LOADER_H_