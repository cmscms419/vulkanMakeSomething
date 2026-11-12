# vulkanMakeSomething_basic2

Vulkan 기반의 렌더링 엔진(basicEngine2)을 개발하는 프로젝트입니다.
RenderDoc을 이용한 셰이더 디버깅도 지원합니다.

처음 목표였던 "DirectX 11에서 구현했던 3D 클로스 시뮬레이션을 Vulkan으로 구현"은
달성했습니다. DX11 판과 달리 CPU가 아니라 **컴퓨트 셰이더에서** 돌아가고,
적분 방식도 VBD(Vertex Block Descent)로 바꿨습니다.
→ [clothSimulation](app/source/clothSimulation/)

## 실행 환경

- [Visual Studio 2022](https://visualstudio.microsoft.com/ko/vs/)
- [CUDA Toolkit 12.6](https://developer.nvidia.com/cuda-12-6-0-download-archive?target_os=Windows&target_arch=x86_64&target_version=11&target_type=exe_network)
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- Windows 11 (Windows 11에서만 실행됨)


## 프로젝트 구조

```
vulkanMakeSomething_basic2/
├── Project/                    # Visual Studio 솔루션 (VKstudio.sln)
│   ├── basicEngine2/           # 핵심 엔진 정적 라이브러리 프로젝트
│   ├── component2/             # 컴포넌트 정적 라이브러리 프로젝트
│   ├── common/                 # 공용 유틸리티 라이브러리 프로젝트
│   └── (예제별 실행 파일 프로젝트는 생략)
├── app/
│   ├── engine2/                # 핵심 Vulkan 엔진 구현 소스
│   ├── component2/             # 렌더러, 윈도우, 카메라, GUI 소스
│   ├── helper/                 # 헬퍼 유틸리티 소스
│   ├── common/                 # 공통 유틸리티 (로그, 수학)
│   └── source/                 # 예제 애플리케이션 소스
├── include/
│   ├── engine2/                # 엔진 헤더
│   ├── component2/             # 컴포넌트 헤더
│   ├── struct/                 # 데이터 구조 헤더
│   └── helper/                 # 헬퍼 헤더
├── external/                   # 외부 라이브러리 (GLFW, Assimp, ImGui 등)
├── shader/                     # GLSL 셰이더 소스 (.spv는 Git 미포함 — compile.bat으로 생성)
└── resource/                   # 텍스처, 모델, 폰트 등 리소스
```

## 빌드 의존성 구조

```
common (정적 라이브러리)
  └── basicEngine2 (정적 라이브러리)
        └── component2 (정적 라이브러리)
              └── 최종 애플리케이션 (예제별 실행 파일)
```

## 외부 라이브러리

| 라이브러리 | 용도 |
|-----------|------|
| Vulkan SDK | GPU 렌더링 API |
| GLFW | 윈도우 생성, 입력 처리 |
| Assimp | OBJ, FBX, GLTF 모델 로딩 |
| ImGui | UI 시스템 |
| tinygltf | GLTF/GLB 포맷 지원 |
| KTX | Khronos Texture Format |
| libpng / zlib | PNG 이미지 처리 |
| SPIRV-Reflect | SPIR-V 셰이더 리플렉션 |
| STB | 이미지 로딩 (stb_image.h) |
| CUDA Toolkit 12.6 | GPU 컴퓨팅(프로젝트만 cuda 설정이 되어 있음, 아직 cuda로 무엇을 하지 않음) | 


## 렌더링 기능

- **Forward PBR** - 물리 기반 렌더링 (Physically Based Rendering)
- **Image Based Lighting (IBL)** - 환경맵 기반 고품질 조명 (Irradiance, Prefilter, BRDF LUT)
- **Shadow Mapping** - 실시간 그림자 생성
- **Skybox** - 큐브맵 기반 스카이박스
- **Post Processing** - 포스트 프로세싱 파이프라인
- **Compute Shader** - 병렬 GPU 컴퓨팅 (SSAO, 디퍼드 라이팅, 천 시뮬레이션)
- **GPU Cloth Simulation (VBD)** - 암시적 적분을 정점별 3x3 뉴턴 풀이로 분해해 컴퓨트 셰이더에서 병렬 처리
- **Graph Coloring** - 이웃끼리 겹치지 않는 그룹으로 나눠 Gauss-Seidel 반복을 병렬화
- **RenderGraph** - JSON으로 패스와 리소스를 선언하고 이미지 배리어를 자동 삽입
- **Skeletal Animation** - Assimp 기반 뼈대 애니메이션
- **ImGui UI** - 실시간 파라미터 조작 UI
- **View Frustum Culling** - 뷰 프러스텀 기반 렌더링 최적화
- **SPIRV-Reflect** - 셰이더 리플렉션을 통한 동적 리소스 바인딩


## Content (basicEngine2 바탕으로)

예제마다 Visual Studio 프로젝트가 하나씩 있습니다 (`Project/<이름>/`).
셰이더를 먼저 컴파일해야 합니다 ([셰이더 컴파일](#셰이더-컴파일) 참고).

`` 표시는 [외부 에셋](#외부-에셋-git-미포함)을 따로 받아야 실행되는 예제입니다.
표시가 없으면 저장소에 있는 리소스만으로 돌아갑니다.

### 시뮬레이션

**[clothSimulation](app/source/clothSimulation/)** — GPU 천 시뮬레이션 (VBD)

24×24 격자(정점 576개, 스프링 3174개)를 컴퓨트 셰이더에서 암시적 적분으로 풉니다.
그래프 컬러링으로 나눈 5개 그룹을 10회 반복해 프레임당 52번 dispatch 합니다.
ImGui로 격자·강성·반복 횟수·중력·바람을 실시간 조절하고, GPU 결과를 CPU로
읽어와 검증할 수 있습니다. 모델을 쓰지 않으므로 추가 에셋이 필요 없습니다.
조작: 마우스 오른쪽 드래그로 시점 회전, `W`/`S`/`A`/`D`/`E`/`Q` 이동, `Shift` 부스트.

**[PhysicalSimulation](app/source/PhysicalSimulation/)**  — 강체 물리 + 인스턴싱

`PhysicsSimulator`로 구를 갱신하고 인스턴스 버퍼 하나로 한 번에 그립니다
(최대 15,000개). `renderGraph_instance_version.json`을 사용합니다.

### 씬 데모 (Application 프레임워크)

**[sampleDemo](app/source/sampleDemo/)**  — 맵 위를 걸어 다니는 캐릭터

바닥 높이 탐색(`findYGround`)과 벽 충돌 처리가 들어간 캐릭터 컨트롤러입니다.
Bistro 씬 위에서 골격 애니메이션이 재생되는 캐릭터를 3인칭으로 따라갑니다.

**[sampleDemo2](app/source/sampleDemo2/)**  — 자유 비행 카메라와 그림자

Sponza 씬(`resource/noGit/sponza/`)을 디퍼드로 그리고 그림자용
`lightSpaceMatrix`를 매 프레임 갱신합니다.
조작: 마우스 오른쪽 드래그로 시점 회전, `W`/`S`/`A`/`D`/`E`/`Q` 이동, `Shift` 부스트.

**[GLTF](app/source/GLTF/)**  — Assimp 모델 로딩

Assimp로 모델을 읽어 디퍼드 PBR로 그립니다. 다만 아직 작성 중입니다 —
`gltf_example.cpp`가 비어 있고 `main.cpp`가 만든 `ApplicationConfig`가
`Application`에 전달되지 않아, 실제로는 기본 씬(Bistro + 캐릭터)이 뜹니다.

### 렌더링 기법 (엔진을 직접 호출)

**[skybox](app/source/skybox/)** — 큐브맵 스카이박스와 IBL

창·스왑체인·카메라·파이프라인을 `Application` 없이 직접 구성합니다.
ImGui로 환경맵 강도, prefiltered 맵의 밉 레벨, irradiance 맵 전환,
밉 레벨/큐브 면 시각화를 조절할 수 있습니다.

**[postProcessing](app/source/postProcessing/)** — HDR 포스트 프로세싱

HDR 색 버퍼에 그린 뒤 포스트 패스에서 톤매핑합니다. Reinhard, ACES,
Uncharted2, GT, Lottes, Hable 등 10가지 연산자와 노출·감마를 비교해 볼 수
있습니다. IBL 큐브맵(`resource/cubeMap/`)을 사용합니다.

**[renderGUI](app/source/renderGUI/)** — ImGui 통합

창과 스왑체인을 띄우고 ImGui를 붙이는 최소 구성입니다.
UI로 클리어 색을 바꿔 가며 UI → 렌더링 경로가 이어졌는지 확인합니다.

### 엔진 기초

**[Pipeline](app/source/Pipeline/)** — 오프스크린 렌더링

스왑체인 없이 이미지에 삼각형을 그려 `output.jpg`로 저장합니다.
결과를 화면이 아니라 파일로 확인하는 경로라, 창이 없는 환경에서도 씁니다.

**[shadeReflect](app/source/shadeReflect/)** — SPIR-V 리플렉션

`.spv`를 읽어 디스크립터 바인딩과 푸시 상수 정보를 로그로 출력합니다.
엔진이 파이프라인 레이아웃을 자동으로 만드는 근거를 눈으로 보는 예제입니다.

**[basic2](app/source/basic2/)** — Vulkan 초기화만

`Context` 하나를 만들고 끝납니다(11줄). 창도 렌더링도 없습니다.
인스턴스와 논리 디바이스 생성이 되는지 확인하는 최소 예제입니다.


## 셰이더 컴파일

`shader/` 디렉토리에서 배치 파일 실행:

```bat
compile.bat          # 전체 셰이더 컴파일
compile_debug.bat    # 디버그 모드 컴파일
```

GLSL 소스(`.vert`, `.frag`, `.comp`)를 SPIR-V 바이너리(`.spv`)로 컴파일합니다.

## 외부 에셋 (Git 미포함)

용량이 큰 에셋은 저장소에 포함되지 않습니다. 아래 안내에 따라 직접 다운로드 후 지정 경로에 배치하세요.

---

### Amazon Lumberyard Bistro

데모 씬의 배경 환경으로 사용됩니다.

**다운로드:** [Morgan McGuire's Computer Graphics Archive](https://casual-effects.com/data/)  
→ 페이지에서 **Bistro** 검색 후 다운로드

**설치 경로:**
```
resource/
└── AmazonLumberyardBistroMorganMcGuire/
    ├── exterior.obj
    ├── exterior.mtl
    ├── interior.obj
    ├── interior.mtl
    └── ... (텍스처 파일들)
```

---

### Leonard 캐릭터 (Mixamo)

골격 애니메이션 캐릭터입니다. [Mixamo](https://www.mixamo.com) 계정 생성 후 무료 다운로드 가능합니다.

**다운로드 방법:**
1. [Mixamo](https://www.mixamo.com) 접속 → 로그인
2. Characters 탭에서 **Leonard** 검색 → 선택
3. Animations 탭에서 아래 목록의 애니메이션 각각 선택
4. **Download** → Format: **FBX**, Skin: **With Skin** → 다운로드

**필요한 애니메이션 목록:**

| 파일명 | Mixamo 검색어 |
|---|---|
| `Idle.fbx` | Idle |
| `Walking.fbx` | Walking |
| `Standard Walk.fbx` | Standard Walk |
| `Start Walking.fbx` | Start Walking |
| `Stop Walking.fbx` | Stop Walking |
| `Bboy Hip Hop Move.fbx` | Bboy Hip Hop Move |
| `Gangnam Style.fbx` | Gangnam Style |
| `Tut Hip Hop Dance.fbx` | Tut Hip Hop Dance |
| `CatwalkIdle.fbx` | Catwalk Idle |
| `Listening To Music.fbx` | Listening To Music |

**설치 경로:**
```
resource/
└── characters/
    └── Leonard/
        ├── Leonard.fbx          ← 캐릭터 메시 (Skin 포함)
        ├── Idle.fbx
        ├── Walking.fbx
        ├── Standard Walk.fbx
        ├── Start Walking.fbx
        ├── Stop Walking.fbx
        ├── Bboy Hip Hop Move.fbx
        ├── Gangnam Style.fbx
        ├── Tut Hip Hop Dance.fbx
        ├── CatwalkIdle.fbx
        └── Listening To Music.fbx
```

---

### Sponza (sampleDemo2)

`sampleDemo2`가 배경으로 사용합니다. glTF 버전이 필요합니다.

**다운로드:** [KhronosGroup/glTF-Sample-Assets](https://github.com/KhronosGroup/glTF-Sample-Assets)
→ `Models/Sponza`

**설치 경로:**
```
resource/
└── noGit/
    └── sponza/
        ├── sponza.gltf
        └── ... (bin, 텍스처 파일들)
```

---

### 기본 도형 (PhysicalSimulation)

`PhysicalSimulation`이 쓰는 정육면체와 구입니다. 배포본이 따로 있는 것이
아니라 직접 내보내 넣으면 됩니다 (Blender 등에서 기본 Cube / Sphere를
각각 OBJ와 glTF로 내보내기).

**설치 경로:**
```
resource/
└── 3D_models/
    ├── Box.obj
    ├── box.mtl
    ├── sphere.gltf
    ├── sphere.bin
    └── ... (텍스처 파일들)
```

> `.gitignore`가 `*.obj`를 무시하므로 이 디렉토리는 저장소에 들어가지
> 않습니다. `clothSimulation`은 바닥 모델을 쓰지 않도록 바꿔서 이 에셋이
> 없어도 실행됩니다.

---

## 텍스처 소스 출처

[텍스처 소스 출처](https://opengameart.org/content/tiny-texture-pack-2)

[텍스처 홈페이지](https://opengameart.org/)

[skymap 텍스처 출처](https://learnopengl.com/Advanced-OpenGL/Cubemaps)

[GLTF-Sample](https://github.com/KhronosGroup/glTF-Sample-Models/tree/main)

## 코드 출처

KhronosGroup에서 개발한 [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect) (common 프로젝트에 포함되어서 static lib로 빌드)

[skybox.vert, skybox.frag 출처](https://github.com/SaschaWillems/Vulkan/tree/master/shaders/glsl/texturecubemap)

[PBR basic 출처, 셰이더 출처](https://github.com/SaschaWillems/Vulkan/tree/master/examples/pbrbasic)

[image base lighting PBR 코드 출처](https://github.com/SaschaWillems/Vulkan/tree/master/examples/pbribl)

[HonglabVulkan](https://github.com/HongLabInc/HonglabVulkan)
