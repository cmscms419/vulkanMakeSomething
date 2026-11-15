#ifndef INCLUDE_COMPONENT2_RAYCAST_H_
#define INCLUDE_COMPONENT2_RAYCAST_H_

#include "common.h"

#include <glm/glm.hpp>

#include <cmath>

namespace vkengine
{
    namespace raycast
    {
        // ---------------------------------------------------------------------
        // 마우스 레이캐스트에 쓰는 수학.
        //
        // intersectRayTriangle / intersectRayPlane 은 GPU 셰이더
        // (clothPick.comp, clothGrabUpdate.comp) 의 기준 구현이기도 하다.
        // 셰이더와 결과가 어긋나면 이쪽을 먼저 믿는다.
        // ---------------------------------------------------------------------

        struct Ray
        {
            cVec3 origin = cVec3(0.0f);
            cVec3 direction = cVec3(0.0f, 0.0f, 1.0f);
        };

        struct RayTriangleHit
        {
            cBool hit = false;
            cFloat distance = 0.0f;                       // origin 에서 교차점까지 (direction 이 단위라 거리와 같다)
            cVec3 barycentric = cVec3(0.0f);              // (w0, w1, w2) — 각 꼭짓점의 무게
        };

        // 화면 좌표(픽셀, 왼쪽 위가 원점)를 월드 공간 레이로 바꾼다.
        //
        // NDC y 는 뒤집지 않는다.
        //   Camera2::setPerspectiveProjection() 이 이미 projection[1][1] *= -1 로
        //   Vulkan 의 아래쪽이 +y 인 화면에 맞춰 두었다. 그래서 화면 맨 위(y = 0)가
        //   그대로 NDC -1 이다. OpenGL 예제처럼 1 - y 를 하면 위아래가 거꾸로 잡힌다.
        //
        // origin 과 direction 을 카메라 위치가 아니라 near(z=0) / far(z=1) 두 점을
        // 되돌려서 만든다. 깊이 범위는 이 엔진의 규약(0..1)을 따르고, 행렬만 있으면
        // 되므로 직교 투영에서도 그대로 맞다.
        inline Ray makeRayFromScreen(cVec2 screenPosition, cVec2 screenSize,
                                     const cMat4& view, const cMat4& projection)
        {
            const cFloat ndcX = 2.0f * screenPosition.x / screenSize.x - 1.0f;
            const cFloat ndcY = 2.0f * screenPosition.y / screenSize.y - 1.0f;

            const cMat4 inverseViewProjection = glm::inverse(projection * view);

            cVec4 nearPoint = inverseViewProjection * cVec4(ndcX, ndcY, 0.0f, 1.0f);
            cVec4 farPoint = inverseViewProjection * cVec4(ndcX, ndcY, 1.0f, 1.0f);
            nearPoint /= nearPoint.w;
            farPoint /= farPoint.w;

            Ray ray{};
            ray.origin = cVec3(nearPoint);
            ray.direction = glm::normalize(cVec3(farPoint) - cVec3(nearPoint));
            return ray;
        }

        // Möller–Trumbore. 양면 판정이다 — 천은 앞뒤 구분 없이 잡혀야 한다.
        // distance 가 0 이하인(레이 뒤쪽) 교차는 맞지 않은 것으로 본다.
        inline RayTriangleHit intersectRayTriangle(const Ray& ray,
                                                   cVec3 p0, cVec3 p1, cVec3 p2)
        {
            RayTriangleHit result{};

            const cVec3 edge1 = p1 - p0;
            const cVec3 edge2 = p2 - p0;
            const cVec3 pVector = glm::cross(ray.direction, edge2);
            const cFloat determinant = glm::dot(edge1, pVector);

            // 레이가 삼각형 면과 평행하거나, 삼각형이 선으로 뭉개진 경우
            if (std::fabs(determinant) < 1e-12f)
            {
                return result;
            }

            const cFloat inverseDeterminant = 1.0f / determinant;
            const cVec3 tVector = ray.origin - p0;

            const cFloat u = glm::dot(tVector, pVector) * inverseDeterminant;
            if (u < 0.0f || u > 1.0f)
            {
                return result;
            }

            const cVec3 qVector = glm::cross(tVector, edge1);
            const cFloat v = glm::dot(ray.direction, qVector) * inverseDeterminant;
            if (v < 0.0f || u + v > 1.0f)
            {
                return result;
            }

            const cFloat distance = glm::dot(edge2, qVector) * inverseDeterminant;
            if (distance <= 0.0f)
            {
                return result;
            }

            result.hit = true;
            result.distance = distance;
            result.barycentric = cVec3(1.0f - u - v, u, v);
            return result;
        }

        // 레이와 평면(점 planePoint, 법선 planeNormal)의 교차 거리.
        // 평행하거나 교차점이 레이 뒤쪽이면 false 를 돌려주고 outDistance 를 건드리지 않는다.
        // 드래그 중에는 이 결과가 false 이면 이전 목표점을 그대로 둔다.
        inline cBool intersectRayPlane(const Ray& ray, cVec3 planePoint, cVec3 planeNormal,
                                       cFloat& outDistance)
        {
            const cFloat denominator = glm::dot(planeNormal, ray.direction);
            if (std::fabs(denominator) < 1e-6f)
            {
                return false;
            }

            const cFloat distance = glm::dot(planePoint - ray.origin, planeNormal) / denominator;
            if (distance <= 0.0f)
            {
                return false;
            }

            outDistance = distance;
            return true;
        }

        inline cVec3 makePointOnRay(const Ray& ray, cFloat distance)
        {
            return ray.origin + ray.direction * distance;
        }
    }
}

#endif // !INCLUDE_COMPONENT2_RAYCAST_H_
