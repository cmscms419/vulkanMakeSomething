#include "struct.h"
#include <glm/gtx/hash.hpp>

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

const std::vector<Vertex> DepthTestVertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}
};

const std::vector<Vertex> SquareTestVertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
};

const std::vector<uint16_t> DepthTestIndices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
};

const std::vector<uint16_t> SquareTestIndices_ = {
        0, 1, 2, 2, 3, 0
};

const std::vector<Vertex> cube{

    // left face (white)
    {{-0.5f, -0.5f, -0.5f}, {.9f, .9f, .9f}, { 0.0f, 0.0f, 0.0f } },
    { {-0.5f, -0.5f, 0.5f}, {.9f, .9f, .9f} , { 1.0f, 0.0f, 0.0f }},
    { {-0.5f, 0.5f, 0.5f}, {.9f, .9f, .9f} , { 1.0f, 1.0f, 0.0f }},
    { {-0.5f, 0.5f, -0.5f}, {.9f, .9f, .9f} , { 0.0f, 1.0f, 0.0f }},

    // right face (yellow)
    { {0.5f, -0.5f, -0.5f}, {.8f, .8f, .1f} , { 0.0f, 0.0f, 1.0f }},
    { {0.5f, 0.5f, -0.5f}, {.8f, .8f, .1f} , { 1.0f, 0.0f, 1.0f }},
    { {0.5f, 0.5f, 0.5f}, {.8f, .8f, .1f} , { 1.0f, 1.0f , 1.0f}},
    { {0.5f, -0.5f, 0.5f}, {.8f, .8f, .1f} , { 0.0f, 1.0f , 1.0f}},

    // top face (orange, remember y axis points down)
    { { -0.5f, -0.5f, -0.5f }, { .9f, .6f, .1f }, { 0.0f, 0.0f, 2.0f } },
    { {0.5f, -0.5f, -0.5f}, {.9f, .6f, .1f} , { 1.0f, 0.0f , 2.0f}},
    { {0.5f, -0.5f, 0.5f}, {.9f, .6f, .1f} , { 1.0f, 1.0f , 2.0f}},
    { {-0.5f, -0.5f, 0.5f}, {.9f, .6f, .1f} , { 0.0f, 1.0f , 2.0f}},

    // bottom face (red)
    { { -0.5f, 0.5f, -0.5f }, { .8f, .1f, .1f }, { 0.0f, 0.0f, 3.0f } },
    { {-0.5f, 0.5f, 0.5f}, {.8f, .1f, .1f} , { 1.0f, 0.0f , 3.0f }},
    { {0.5f, 0.5f, 0.5f}, {.8f, .1f, .1f} , { 1.0f, 1.0f , 3.0f }},
    { {0.5f, 0.5f, -0.5f}, {.8f, .1f, .1f} , { 0.0f, 1.0f , 3.0f }},

    // nose face (blue)
    { { -0.5f, -0.5f, 0.5f }, { .1f, .1f, .8f }, { 0.0f, 0.0f, 4.0f  } },
    { {0.5f, -0.5f, 0.5f}, {.1f, .1f, .8f} , { 1.0f, 0.0f , 4.0f}},
    { {0.5f, 0.5f, 0.5f}, {.1f, .1f, .8f} , { 1.0f, 1.0f , 4.0f}},
    { {-0.5f, 0.5f, 0.5f}, {.1f, .1f, .8f} , { 0.0f, 1.0f , 4.0f}},

    // tail face (green)
    { { -0.5f, -0.5f, -0.5f }, { .1f, .8f, .1f }, { 0.0f, 0.0f , 5.0f} },
    { {-0.5f, 0.5f, -0.5f}, {.1f, .8f, .1f} , { 1.0f, 0.0f , 5.0f}},
    { {0.5f, 0.5f, -0.5f}, {.1f, .8f, .1f} , { 1.0f, 1.0f , 5.0f}},
    { {0.5f, -0.5f, -0.5f}, {.1f, .8f, .1f} , { 0.0f, 1.0f , 5.0f}},
};

const std::vector<uint16_t> cubeindices_ = {
       0, 1, 2, 2, 3, 0,
       4, 5, 6, 6, 7, 4,
       8, 9, 10, 10, 11, 8,
       12, 13, 14, 14, 15, 12,
       16, 17, 18, 18, 19, 16,
       20, 21, 22, 22, 23, 20
};

