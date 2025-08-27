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
            // ±¸ »ý¼º
            static void createSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float radius = 1.0f, uint32_t rings = 16, uint32_t sectors = 16);
        };
    }
}

#endif // INCLUDE_VK_GEOMETRYGENERATOR_H_