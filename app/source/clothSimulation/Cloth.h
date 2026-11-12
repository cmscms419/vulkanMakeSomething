#ifndef CLOTH_H_
#define CLOTH_H_

#include "Application.h"
#include "ClothSimulation.h"
#include "DataManagement.h"

namespace vkengine
{
    /**
     * @brief Vertex Block Descent 기반 천 시뮬레이션 데모.
     *
     * 현재 범위
     *  - ClothSimulation 의 CPU 데이터 생성과 검증
     *  - ClothDataManagement 로 버퍼/파이프라인 생성과 매 프레임 compute dispatch
     *  - 천 렌더링 (Renderer 의 cloth 패스가 compute 결과를 정점 버퍼로 읽는다)
     *  - ImGui 로 격자/스프링/그룹 구성, dispatch/배리어 횟수, 결과 읽어오기
     *    그리고 재빌드가 필요 없는 값의 실시간 조절
     *    (iterations / h / gravity / floorY / 바람)
     *  - 렌더 그래프는 renderGraph_cloth.json (skybox 포함)
     *
     * 아직 없는 것
     *  - 천 머티리얼(색/거칠기)의 ImGui 조절, 정점 법선, 그림자 패스에 천 포함
     *  - 바닥 충돌이 clothFinalize.comp 의 위치 클램프뿐이다 (에너지 항이 아니다)
     *
     * 조작
     *  - 마우스 오른쪽 버튼 드래그 : 시점 회전
     *  - W / S : 전/후진,  A / D : 좌/우,  E / Q : 상/하
     *  - Left Shift : 부스트,  Left Alt : 감속
     */
    class Cloth : public Application
    {
    public:
        Cloth(cString root_path);
        Cloth(const ApplicationConfig& config, cString root_path);
        ~Cloth();

        virtual void update() override;
        virtual void updateGui() override;

        // Application::run() 과 같지만, 프레임 커맨드 버퍼의 맨 앞에
        // 천 compute 를 기록한다. RenderGraph 는 이미지 리소스만 알고 버퍼는
        // 모르므로 패스로 등록할 수 없고, renderGraph.execute() 앞에 직접 넣는다.
        virtual void run() override;

    private:
        void initCamera();
        void initCloth();

        void updateCameraLook();
        void updateCameraMove(cFloat deltaTime);
        void updateLightSpaceMatrix();

        void renderClothControlGUI();

        // cloth 데이터를 다시 만들고 GPU 버퍼까지 다시 올린다.
        // GPU 가 그 버퍼를 쓰고 있을 수 있으므로 먼저 기다린다.
        void rebuildCloth(const ClothSimulation::Info& info);

        // 재빌드가 필요 없는 값(iterations, timeStep, gravity, floorY, 바람)만
        // clothData 에 밀어넣는다. pendingInfo 와 wind* 가 이 값들의 기준이다.
        void applyLiveParams();

        // 돌풍의 위상을 진행시키고 UBO 를 다시 올린다. 매 프레임 불린다.
        void updateWind(cFloat deltaTime);

        // 지금 순간의 돌풍 배율. 돌풍이 꺼져 있으면 1 이다.
        // applyLiveParams() 와 ImGui 표시가 같은 식을 쓰도록 여기 모아 둔다.
        cFloat getGustFactor() const;

        // 디버그 — GPU 의 정점을 CPU 로 가져와 통계를 갱신한다.
        void readbackCloth();

        ClothSimulation cloth;
        ClothDataManagement clothData;

        // 슬라이더가 바로 재생성을 일으키지 않도록 분리한다.
        // 격자 크기나 스프링 종류를 바꾸면 그룹 분할을 다시 계산해야 하는데,
        // 드래그 중 매 프레임 다시 계산하면 프레임이 끊긴다.
        ClothSimulation::Info pendingInfo{};
        cBool clothDirty = false;

        cBool simulationPaused = false;

        // ---- 바람 ---------------------------------------------------------
        // 일정한 힘 F 는 보존력이다 (E = -F.x). 그래서 G 에 그냥 들어가고,
        // 가속도로 바꿔 중력 벡터에 더하는 것으로 끝난다.
        //
        //   g_effective = gravity + normalize(windDirection) * windStrength * gust
        //
        // 셰이더도 버퍼도 배리어도 바뀌지 않는다. UBO 의 vec3 하나뿐이다.
        //
        // 그리고 이것만으로 jitter Z 를 대신할 수 있다. 천이 두 모서리에만
        // 매달려 있어서, 균일한 힘을 주어도 핀 근처와 아래쪽이 다르게 밀린다.
        // 그 순간 z 가 공간적으로 달라지고 헤시안의 xz/yz 결합이 0 이 아니게
        // 되어, 면내 압축이 면 밖으로 새어나갈 통로가 열린다.
        // (CPU 실험: 바람만 켠 쪽의 주름 크기가 jitter 만 켠 쪽과 같았고,
        //  둘을 같이 켜도 바람만 켠 것과 차이가 없었다)

        // 켜고 끄기. 끄면 방향과 세기는 그대로 둔 채 바람만 0 이 된다.
        // "중력 0 + 바람 끔 -> 천이 전혀 움직이지 않는다" 검증을
        // 슬라이더 값을 잃지 않고 바로 해 볼 수 있다.
        cBool windEnabled = false;

        // 세기 (m/s^2). 가속도라서 중력(9.8)과 같은 단위로 비교하면 된다.
        // 2.0 이 중력의 1/5 쯤으로 적당하고, 20 이면 천이 거의 수평으로
        // 날아가면서 스프링도 눈에 띄게 늘어난다.
        cFloat windStrength = 2.0f;

        // 부는 방향. 길이는 의미가 없다 - 쓸 때 정규화한다.
        // 세기와 방향을 나눠 두면 세기만 올려도 부는 쪽이 바뀌지 않는다.
        cVec3 windDirection = cVec3(0.0f, 0.0f, 1.0f);

        // 돌풍. 세기를 sin 으로 흔든다.
        // 시간에 따라 변해도 '한 타임 스텝 안에서는 상수' 이므로 G 의 형태가
        // 바뀌지 않는다. 매 프레임 UBO 를 덮어쓰는 것만으로 공짜로 된다.
        cBool windGust = false;
        cFloat windGustSpeed = 1.3f;    // rad/s
        cFloat windGustAmount = 0.4f;   // 0 = 일정, 1 = 0배 ~ 2배 사이로 흔들린다
        cFloat windTime = 0.0f;         // 돌풍 위상 (초)

        // 시작 직후 한 번만 자동으로 결과를 읽어 로그에 남긴다.
        // 창을 띄우지 않고도 compute 가 돌았는지 확인할 수 있어야 한다.
        static const cUint32_t kAutoReadbackFrame = 120u;
        cBool autoReadbackDone = false;

        // 읽어온 결과 통계 (ImGui 표시용)
        std::vector<ClothVertex> readbackVertices;
        cBool hasReadback = false;
        cFloat readbackMinY = 0.0f;
        cFloat readbackMaxY = 0.0f;
        cVec3 readbackCenter = cVec3(0.0f);
        cUint32_t readbackFrame = 0;
        cUint32_t readbackNanCount = 0;
    };
}

#endif // !CLOTH_H_
