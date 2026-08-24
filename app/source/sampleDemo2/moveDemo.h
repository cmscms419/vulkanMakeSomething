#ifndef RESUME_DEMO_H_
#define RESUME_DEMO_H_

#include "Application.h"

namespace vkengine
{
    /**
     * @brief 자유 비행(Free-fly) 카메라 데모.
     *
     * 조작
     *  - 마우스 오른쪽 버튼 드래그 : 시점 회전 (yaw / pitch)
     *  - W / S                    : 시선 방향 전/후진
     *  - A / D                    : 좌/우 스트레이프
     *  - E / Q                    : 월드 상/하 이동
     *  - Left Shift               : 부스트(가속)
     */
    class DemoApplication : public Application
    {
    public:
        DemoApplication(cString root_path);
        DemoApplication(const ApplicationConfig& config, cString root_path);
        ~DemoApplication();

        virtual void update() override;

    private:
        void setupCamera();                      // 시작 위치 / near·far 구성
        void updateCameraLook();                 // 마우스 회전 처리
        void updateCameraMove(cFloat deltaTime); // 키보드 이동 처리
        void updateLightSpaceMatrix();           // 그림자용 lightSpaceMatrix 갱신
    };
}

#endif // RESUME_DEMO_H_
