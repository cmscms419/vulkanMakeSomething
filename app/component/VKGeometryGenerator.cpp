#include "VKGeometryGenerator.h"

namespace vkengine
{
    namespace helper {
        void GeometryGenerator::createSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float radius, uint32_t rings, uint32_t sectors)
        {
            vertices.clear();
            indices.clear();

            float x, y, z, xy;                              // vertex position
            float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
            float s, t;                                     // vertex texCoord

            float sectorStep = XM_2PI / rings;
            float stackStep = XM_PI / sectors;
            float sectorAngle, stackAngle;

            // Vertex 생성
            for (int i = 0; i <= sectors; ++i)
            {
                stackAngle = XM_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
                xy = radius * cosf(stackAngle);                 // r * cos(u)
                z = radius * sinf(stackAngle);                  // r * sin(u)

                // add (rings+1) vertices per stack
                // first and last vertices have same position and normal, but different tex coords
                for (int j = 0; j <= rings; ++j)
                {
                    Vertex vertex;
                    sectorAngle = j * sectorStep;               // starting from 0 to 2pi

                    // vertex position (x, y, z)
                    x = xy * cosf(sectorAngle);                 // r * cos(u) * cos(v)
                    y = xy * sinf(sectorAngle);                 // r * cos(u) * sin(v)
                    
                    vertex.pos = cVec3(x, y, z);

                    vertex.normal = glm::normalize(cVec3(x, y, z)); // 정규화된 법선 벡터


                    // vertex tex coord (s, t) range between [0, 1]
                    s = (float)j / rings;
                    t = (float)i / sectors;
                    vertex.texCoord = cVec3(s, t, 0.0f);

                    // tangent 계산 (구면의 tangent는 longitude 방향)
                    float tx = -sinf(sectorAngle);
                    float ty = cosf(sectorAngle);
                    float tz = 0.0f;
                    float tw = 1.0f; // tangent w component, typically 0 for sphere

                    //vertex.normal = cVec3(nx, ny, nz);


                    vertex.inTangent = cVec4(tx, ty, tz, tw);

                    vertices.push_back(vertex);
                }
            }

            // 인덱스 생성
            uint32_t k1, k2;
            for (int i = 0; i < sectors; ++i)
            {
                k1 = i * (rings + 1);     // beginning of current stack
                k2 = k1 + rings + 1;      // beginning of next stack

                for (int j = 0; j < rings; ++j, ++k1, ++k2)
                {
                    // 2 triangles per sector excluding first and last stacks
                    // k1 => k2 => k1+1
                    if (i != 0)
                    {
                        indices.push_back(k1);
                        indices.push_back(k2);
                        indices.push_back(k1 + 1);
                    }

                    // k1+1 => k2 => k2+1
                    if (i != (sectors - 1))
                    {
                        indices.push_back(k1 + 1);
                        indices.push_back(k2);
                        indices.push_back(k2 + 1);
                    }
                }
            }

        }
    }
}

