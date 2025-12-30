#include "VKViewFrustum.h"

namespace vkengine
{
    cBool ViewFrustum::intersects(const AABB& aabb) const
    {
        for (const auto& plane : planes) {
            // Find the positive vertex (farthest in direction of plane normal)
            glm::vec3 positiveVertex = aabb.min;
            if (plane.normal.x >= 0)
                positiveVertex.x = aabb.max.x;
            if (plane.normal.y >= 0)
                positiveVertex.y = aabb.max.y;
            if (plane.normal.z >= 0)
                positiveVertex.z = aabb.max.z;

            // If positive vertex is behind plane, AABB is completely outside
            if (plane.distanceToPoint(positiveVertex) < 0) {
                return false;
            }
        }
        return true; // AABB is either inside or intersecting
    }
    cBool ViewFrustum::contains(const glm::vec3& point) const
    {
        for (const auto& plane : planes) {
            if (plane.distanceToPoint(point) < 0) {
                return false;
            }
        }
        return true;
    }
    void ViewFrustum::extractFromViewProjection(const glm::mat4& viewProjection)
    {
        // Extract frustum planes from view-projection matrix
    // Use row-major indexing since GLM uses column-major storage
        const float* m = &viewProjection[0][0];

        // Left plane: m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]
        planes[sLEFT].normal.x = m[3] + m[0];
        planes[sLEFT].normal.y = m[7] + m[4];
        planes[sLEFT].normal.z = m[11] + m[8];
        planes[sLEFT].distance = m[15] + m[12];

        // Right plane: m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]
        planes[sRIGHT].normal.x = m[3] - m[0];
        planes[sRIGHT].normal.y = m[7] - m[4];
        planes[sRIGHT].normal.z = m[11] - m[8];
        planes[sRIGHT].distance = m[15] - m[12];

        // Bottom plane: m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]
        planes[sBOTTOM].normal.x = m[3] + m[1];
        planes[sBOTTOM].normal.y = m[7] + m[5];
        planes[sBOTTOM].normal.z = m[11] + m[9];
        planes[sBOTTOM].distance = m[15] + m[13];

        // Top plane: m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]
        planes[sTOP].normal.x = m[3] - m[1];
        planes[sTOP].normal.y = m[7] - m[5];
        planes[sTOP].normal.z = m[11] - m[9];
        planes[sTOP].distance = m[15] - m[13];

        // Near plane: m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]
        planes[sNEAR].normal.x = m[3] + m[2];
        planes[sNEAR].normal.y = m[7] + m[6];
        planes[sNEAR].normal.z = m[11] + m[10];
        planes[sNEAR].distance = m[15] + m[14];

        // Far plane: m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]
        planes[sFAR].normal.x = m[3] - m[2];
        planes[sFAR].normal.y = m[7] - m[6];
        planes[sFAR].normal.z = m[11] - m[10];
        planes[sFAR].distance = m[15] - m[14];

        // Normalize all planes
        for (auto& plane : planes) {
            float length = glm::length(plane.normal);
            if (length > 0.0f) {
                plane.normal /= length;
                plane.distance /= length;
            }
        }
    }

}
