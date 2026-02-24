#include "geometry.h"

Plane::Plane(const cVec3& p1, const cVec3& p2, const cVec3& p3)
{
    cVec3 v1 = p2 - p1;
    cVec3 v2 = p3 - p1;
    normal = glm::normalize(glm::cross(v1, v2));
    distance = -glm::dot(normal, p1);
}

Plane::Plane(const cVec3& normal, const cVec3& point) 
    : normal(glm::normalize(normal))
{
    distance = -glm::dot(this->normal, point);
}

cFloat Plane::distanceToPoint(const cVec3& point) const
{
    return glm::dot(normal, point) + distance;
}

AABB AABB::transform(const cMat4& matrix) const
{
    // Transform all 8 corners and find new min/max
    cVec3 corners[8] = { {min.x, min.y, min.z}, {max.x, min.y, min.z}, {min.x, max.y, min.z},
                            {max.x, max.y, min.z}, {min.x, min.y, max.z}, {max.x, min.y, max.z},
                            {min.x, max.y, max.z}, {max.x, max.y, max.z} };

    cVec3 newMin(FLT_MAX);
    cVec3 newMax(-FLT_MAX);

    for (int i = 0; i < 8; ++i) {
        cVec4 transformed = matrix * cVec4(corners[i], 1.0f);
        cVec3 point = cVec3(transformed) / transformed.w;

        newMin = (glm::min)(newMin, point);
        newMax = (glm::max)(newMax, point);
    }

    return AABB(newMin, newMax);
}