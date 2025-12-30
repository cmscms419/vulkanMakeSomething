#include "VKModelNode.h"
#include "VKMaterial.h"
#include "VKMesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace vkengine {

void VKModelNode::updateLocalMatrix()
{
    cMat4 translationMatrix = translate(cMat4(1.0f), translation);
    cMat4 rotationMatrix = mat4_cast(rotation);
    cMat4 scaleMatrix = glm::scale(cMat4(1.0f), scale);

    localMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}

void VKModelNode::updateWorldMatrix(const cMat4 &parentMatrix)
{
    worldMatrix = parentMatrix * localMatrix;

    for (auto &child : children) {
        child->updateWorldMatrix(worldMatrix);
    }
}

VKModelNode *VKModelNode::findNode(const cString&name)
{
    if (this->name == name) {
        return this;
    }

    for (auto &child : children) {
        VKModelNode *result = child->findNode(name);
        if (result) {
            return result;
        }
    }

    return nullptr;
}

} // namespace vkengine
