#ifndef INCLUDE_GEOMETRY_H_
#define INCLUDE_GEOMETRY_H_

#include "common.h"

struct Plane
{
    cVec3 normal;
    cFloat distance; 

    Plane() = default;
    Plane(const cVec3& p1, const cVec3& p2, const cVec3& p3);
    Plane(const cVec3& normal, const cVec3& point);

    cFloat distanceToPoint(const cVec3& point) const;
};

struct AABB
{
    cVec3 min = cVec3(0.0f);
    cVec3 max = cVec3(0.0f);

    AABB() = default;
    AABB(const cVec3& min_, const cVec3& max_) : min(min_), max(max_)
    {
    }

    cVec3 getCenter() const
    {
        return (min + max) * 0.5f;
    }
    cVec3 getExtents() const
    {
        return (max - min) * 0.5f;
    }

    // Transform AABB by matrix
    AABB transform(const cMat4& matrix) const;
};

#endif // !INCLUDE_GEOMETRY_H_