#ifndef VK_VIEW_FRUSTUM_INCLUDE_H_
#define VK_VIEW_FRUSTUM_INCLUDE_H_

#include "common.h"
#include "struct.h"
#include "log.h"

namespace vkengine
{
    // ViewFrustum은 3D 그래픽스 렌더링에서 뷰 절두체(View Frustum) 컬링을 구현하는 클래스입니다. 
    // 카메라가 볼 수 있는 공간을 정의하고, 특정 객체가 카메라 시야 내에 있는지 판단하여 렌더링 성능을 최적화합니다.
    class ViewFrustum
    {
    public:
        enum PlaneIndex { sLEFT = 0, sRIGHT = 1, sBOTTOM = 2, sTOP = 3, sNEAR = 4, sFAR = 5 };

        ViewFrustum() = default;

        /**
         * @brief View-Projection 행렬로부터 6개의 절두체 평면을 추출합니다.
         *
         * @details Gribb-Hartmann 평면 추출 알고리즘을 사용합니다.
         *          View-Projection 행렬의 행(row)을 조합하여 각 평면의 방정식 (Ax + By + Cz + D = 0)을
         *          계산하고 정규화합니다.
         *
         * @param viewProjection 카메라의 View * Projection 변환 행렬
         *
         * @note 매 프레임마다 카메라가 이동하거나 회전할 때 호출해야 합니다.
         *
         * @algorithm
         *   행렬 m의 인덱스: m[row * 4 + col]
         *   - Left plane:   4th row + 1st row
         *   - Right plane:  4th row - 1st row
         *   - Bottom plane: 4th row + 2nd row
         *   - Top plane:    4th row - 2nd row
         *   - Near plane:   4th row + 3rd row
         *   - Far plane:    4th row - 3rd row
         */
        void extractFromViewProjection(const glm::mat4& viewProjection);

        /**
        * @brief AABB(Axis-Aligned Bounding Box)가 뷰 절두체와 교차하는지 검사합니다.
        *
        * @details Positive Vertex Test 알고리즘을 사용하여 효율적으로 컬링을 수행합니다.
        *          각 평면에 대해 AABB의 가장 먼 꼭짓점을 찾아 절두체 외부에 있는지 확인합니다.
        *
        * @param aabb 검사할 객체의 바운딩 박스
        * @return true: AABB가 절두체 내부 또는 교차, false: 완전히 외부
        *
        * @note 이 메서드는 프레임당 수천 번 호출될 수 있으므로 성능이 중요합니다.
        */
        cBool intersects(const AABB& aabb) const;

        /**
         * @brief 특정 점이 뷰 절두체 내부에 있는지 검사합니다.
         *
         * @details 6개의 평면 모두에 대해 점이 앞쪽(양수)에 있는지 확인합니다.
         *          하나라도 뒤쪽에 있으면 절두체 외부로 판단합니다.
         *
         * @param point 검사할 3D 공간 상의 점
         * @return true: 점이 절두체 내부, false: 외부
         *
         * @note 주로 라이트 소스, 파티클, 카메라 위치 등 단일 점 검사에 사용됩니다.
         */
        cBool contains(const glm::vec3& point) const;

    private:
        std::array<Plane, 6> planes{};
    };
}

#endif // !VK_VIEW_FRUSTUM_INCLUDE_H_
