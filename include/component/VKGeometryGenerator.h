#ifndef INCLUDE_VK_GEOMETRYGENERATOR_H_
#define INCLUDE_VK_GEOMETRYGENERATOR_H_

#include "common.h"
#include "struct.h"
#include "macros.h"
#include "vkmath.h"

namespace vkengine {
    namespace helper {
        class GeometryGenerator {
        public:
            GeometryGenerator() = default;
            ~GeometryGenerator() = default;
            // ������ü ����
            static void createCube(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, float size = 1.0f);
            // �� ����
            static void createSphere(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, float radius = 1.0f, uint32_t rings = 16, uint32_t sectors = 16);
            // ��� ����
            static void createPlane(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, float width = 1.0f, float height = 1.0f);
        };
    }
}

#endif // INCLUDE_VK_GEOMETRYGENERATOR_H_