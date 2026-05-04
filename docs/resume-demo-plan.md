# Resume Demo 구현 계획

## 목표
이력서용 데모: 3D 환경 안에서 3D 캐릭터들이 실제로 이동하고 애니메이션 재생.

---

## 씬 구성

| 역할 | 파일 | 위치 | 동작 |
|---|---|---|---|
| 걷는 캐릭터 | `Walking.fbx` | 원형 경로로 이동 | 매 프레임 modelMatrix 갱신 |
| 댄서 1 | `Gangnam Style.fbx` | 고정 위치 | 강남스타일 루프 재생 |
| 댄서 2 | `Bboy Hip Hop Move.fbx` | 고정 위치 | 비보이 댄스 루프 재생 |
| 환경 | `exterior.obj` (Bistro) | 씬 전체 | 정적 |

---

## 필요한 변경사항

### 1. `include/component2/Application.h` (수정)

`models`를 `private` → `protected`로 이동,
`virtual void onUpdate(float deltaTime)` protected 메서드 추가.

```cpp
protected:
    std::vector<VKModel> models;
    virtual void onUpdate(float deltaTime) {}

private:
    // ... 나머지 기존 멤버들 ...
```

---

### 2. `app/component2/Application.cpp` (수정)

`update()` 루프 내 애니메이션 갱신 이후에 `onUpdate` 호출 추가 (약 278번째 줄):

```cpp
for (auto &model : models)
{
    if (model.hasAnimations())
    {
        model.updateAnimation(deltaTime);
    }
}

// 추가: 서브클래스에서 모델 이동 등 커스텀 업데이트
this->onUpdate(deltaTime);
```

---

### 3. `include/struct/vkconfig.h` (수정)

`ApplicationConfig`에 `createResumeDemo()` 정적 메서드 추가:

```cpp
static ApplicationConfig createResumeDemo()
{
    ApplicationConfig config;

    // 캐릭터 0: 걸어다니는 캐릭터 (ResumeDemo::onUpdate에서 위치 갱신)
    ModelConfig walker("characters/Leonard/Walking.fbx", "Leonard_Walker");
    walker.transform = glm::scale(
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.375f, 0.0f)),
        glm::vec3(0.012f));
    walker.autoPlayAnimation = true;

    // 캐릭터 1: 강남스타일 댄서
    ModelConfig dancer1("characters/Leonard/Gangnam Style.fbx", "Leonard_Gangnam");
    dancer1.transform = glm::rotate(
        glm::scale(
            glm::translate(glm::mat4(1.0f), glm::vec3(-3.5f, 0.375f, -2.0f)),
            glm::vec3(0.012f)),
        glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    dancer1.autoPlayAnimation = true;

    // 캐릭터 2: 비보이 댄서
    ModelConfig dancer2("characters/Leonard/Bboy Hip Hop Move.fbx", "Leonard_Bboy");
    dancer2.transform = glm::rotate(
        glm::scale(
            glm::translate(glm::mat4(1.0f), glm::vec3(3.5f, 0.375f, -2.0f)),
            glm::vec3(0.012f)),
        glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    dancer2.autoPlayAnimation = true;

    // 환경: Bistro exterior
    ModelConfig world("AmazonLumberyardBistroMorganMcGuire/exterior.obj", "Bistro",
                      glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)), true);
    world.autoPlayAnimation = false;

    config.models = {walker, dancer1, dancer2, world};
    return config;
}
```

---

### 4. `app/source/ResumeDemo/ResumeDemo.h` (신규)

```cpp
#ifndef RESUME_DEMO_H_
#define RESUME_DEMO_H_

#include "Application.h"

namespace vkengine {

class ResumeDemo : public Application
{
public:
    ResumeDemo(cString root_path);

protected:
    void onUpdate(float deltaTime) override;

private:
    float walkTime = 0.0f;
};

} // namespace vkengine

#endif
```

---

### 5. `app/source/ResumeDemo/ResumeDemo.cpp` (신규)

```cpp
#include "ResumeDemo.h"

#include <glm/gtc/matrix_transform.hpp>

namespace vkengine {

ResumeDemo::ResumeDemo(cString root_path)
    : Application(ApplicationConfig::createResumeDemo(), root_path)
{
}

void ResumeDemo::onUpdate(float deltaTime)
{
    if (models.empty()) return;

    walkTime += deltaTime;

    // models[0]: 원형 경로로 이동하는 걷는 캐릭터
    const float radius = 4.0f;
    const float speed  = 0.4f;   // 라디안/초
    float angle = walkTime * speed;

    glm::vec3 pos(
        radius * std::cos(angle),
        0.375f,
        radius * std::sin(angle)
    );

    // 접선 방향으로 캐릭터가 바라보도록 (원의 접선 = 속도 방향)
    glm::vec3 forward(-std::sin(angle), 0.0f, std::cos(angle));
    float facingAngle = std::atan2(forward.x, forward.z);

    // Leonard 모델은 기본적으로 -90도 보정 필요
    float totalAngle = facingAngle + glm::radians(-90.0f);

    glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), totalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.012f));

    models[0].ModelResource().modelMatrix = T * R * S;
}

} // namespace vkengine
```

---

### 6. `app/source/ResumeDemo/main.cpp` (신규)

```cpp
#include "ResumeDemo.h"
#include "log.h"

using namespace vkengine;
using namespace vkengine::Log;

int main(int argc, char* argv[])
{
    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    } else {
        EXIT_TO_LOGGER("경로를 가져오는 데 실패했습니다.");
    }

    ResumeDemo demo(root_path);
    demo.update();

    return 0;
}
```

---

### 7. `Project/07.ResumeDemo/07.ResumeDemo.vcxproj` (신규)

기존 `06.GLTF.vcxproj`를 복사 후 아래 항목만 수정:

| 항목 | 06.GLTF 값 | 07.ResumeDemo 값 |
|---|---|---|
| `ProjectGuid` | `{bcbe19c8-...}` | 새 GUID 생성 |
| `RootNamespace` | `_06_GLTF` | `_07_ResumeDemo` |
| `ProjectName` | `06.GLTF` | `07.ResumeDemo` |
| `ClCompile` Include 목록 | `GLTF/gltf_example.cpp`, `GLTF/main.cpp` | `ResumeDemo/ResumeDemo.cpp`, `ResumeDemo/main.cpp` |
| `ClInclude` Include | `GLTF/gltf_example.h` | `ResumeDemo/ResumeDemo.h` |

---

## 구현 순서

```
1. Application.h 수정
2. Application.cpp 수정
3. vkconfig.h 수정
4. app/source/ResumeDemo/ 디렉토리 생성
5. ResumeDemo.h, ResumeDemo.cpp, main.cpp 작성
6. 07.ResumeDemo.vcxproj 생성 (06.GLTF.vcxproj 복사/수정)
7. VKstudio.sln에 07.ResumeDemo 프로젝트 추가
8. 빌드 및 테스트
```

---

## 주의사항

- `models[0]`이 걷는 캐릭터여야 하므로 `createResumeDemo()`의 모델 순서를 지켜야 함
- Leonard 캐릭터의 기본 forward 방향: FBX 기준 조정 필요 시 `facingAngle`에서 `-90도` 보정값 변경
- Bistro exterior 바닥 y 기준: 기존 default config 기준 `y = 0.375f` 사용
- 원형 경로 반지름 `radius = 4.0f`는 Bistro 스케일에 맞게 테스트 후 조정 가능
