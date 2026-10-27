#include "ModelNode.h"
#include "Material.h"
#include "Mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace vkengine {

void ModelNode::updateLocalMatrix()
{
    cMat4 translationMatrix = translate(cMat4(1.0f), translation);
    cMat4 rotationMatrix = mat4_cast(rotation);
    cMat4 scaleMatrix = glm::scale(cMat4(1.0f), scale);

    localMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}

void ModelNode::updateWorldMatrix(const cMat4 &parentMatrix)
{
    worldMatrix = parentMatrix * localMatrix;

    for (auto &child : children) {
        child->updateWorldMatrix(worldMatrix);
    }
}

ModelNode *ModelNode::findNode(const cString&name)
{
    if (this->name == name) {
        return this;
    }

    for (auto &child : children) {
        ModelNode *result = child->findNode(name);
        if (result) {
            return result;
        }
    }

    return nullptr;
}

} // namespace vkengine
