#ifndef VK_MODEL_NODE_INCLUDE_H_
#define VK_MODEL_NODE_INCLUDE_H_

#include "common.h"
#include "struct.h"
#include <memory>

namespace vkengine {

class Mesh;
class VKMaterial;

class VKModelNode
{
  public:
    cString name;
    cMat4 localMatrix = cMat4(1.0f);
    cMat4 worldMatrix = cMat4(1.0f);

    // References to meshes in the parent VKModel
    std::vector<cUint32_t> meshIndices;
    std::vector<std::unique_ptr<VKModelNode>> children;
    VKModelNode *parent = nullptr;

    cVec3 translation = cVec3(0.0f);
    cQuat rotation = cQuat(1.0f, 0.0f, 0.0f, 0.0f);
    cVec3 scale = cVec3(1.0f);

    void updateLocalMatrix();
    void updateWorldMatrix(const cMat4 &parentMatrix = cMat4(1.0f));
    VKModelNode *findNode(const cString &name);

  private:
};

} // namespace vkengine

#endif // !VK_MODEL_NODE_INCLUDE_H_
