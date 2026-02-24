#ifndef INCLUDE_OBJECT_H_
#define INCLUDE_OBJECT_H_

#include "common.h"

// 프리미티브는 단일 드로우 콜에 대한 데이터를 포함합니다.
struct Primitive {
    cUint32_t firstIndex;
    cUint32_t indexCount;
    cInt32_t materialIndex;
};

// 노드의 (선택 사항) 기하학을 포함하고 임의의 수의 기본 요소로 구성될 수 있습니다.
struct Mesh {
    std::vector<::Primitive> primitives;
};

// A node represents an object in the glTF scene graph
struct Node {
    Node* parent;
    std::vector<Node*> children;
    Mesh mesh;
    cMat4 matrix;
    ~Node() {
        for (auto& child : children) {
            delete child;
        }
    }
};


#endif // !INCLUDE_OBJECT_H_