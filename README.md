# vulkanMakeSomething_basic2

Vulkan 기반의 렌더링 엔진(basicEngine2)을 개발하는 프로젝트입니다.
DirectX 11에서 구현했던 3D 클로스 시뮬레이션을 Vulkan으로 구현하는 것이 최종 목표입니다.
RenderDoc을 이용한 셰이더 디버깅도 지원합니다.

## 실행 환경

- [Visual Studio 2022](https://visualstudio.microsoft.com/ko/vs/)
- [CUDA Toolkit 12.6](https://developer.nvidia.com/cuda-12-6-0-download-archive?target_os=Windows&target_arch=x86_64&target_version=11&target_type=exe_network)
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- Windows 11 (Windows 11에서만 실행됨)

## 정리한 개인자료

https://www.notion.so/Vulkan-Tutorial-18118a41dc6f80c7adc7d7ba3f95542e?pvs=4


## 프로젝트 구조

```
vulkanMakeSomething_basic2/
├── Project/                    # Visual Studio 솔루션 (VKstudio.sln)
│   ├── basicEngine2/           # 핵심 엔진 정적 라이브러리 프로젝트
│   ├── component2/             # 컴포넌트 정적 라이브러리 프로젝트
│   └── common/                 # 공용 유틸리티 라이브러리 프로젝트
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
├── shader/                     # GLSL 셰이더 소스 및 SPIR-V 바이너리
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
| CUDA Toolkit 12.6 | GPU 컴퓨팅 |
| SPIRV-Reflect | SPIR-V 셰이더 리플렉션 |
| STB | 이미지 로딩 (stb_image.h) |


## 렌더링 기능

- **Forward PBR** - 물리 기반 렌더링 (Physically Based Rendering)
- **Image Based Lighting (IBL)** - 환경맵 기반 고품질 조명 (Irradiance, Prefilter, BRDF LUT)
- **Shadow Mapping** - 실시간 그림자 생성
- **Skybox** - 큐브맵 기반 스카이박스
- **Post Processing** - 포스트 프로세싱 파이프라인
- **Compute Shader** - 병렬 GPU 컴퓨팅 (SSAO 등)
- **Skeletal Animation** - Assimp 기반 뼈대 애니메이션
- **ImGui UI** - 실시간 파라미터 조작 UI
- **View Frustum Culling** - 뷰 프러스텀 기반 렌더링 최적화
- **SPIRV-Reflect** - 셰이더 리플렉션을 통한 동적 리소스 바인딩


## Content (basicEngine2 바탕으로)

[basic2 - 기본 렌더링 예제](app/source/basic2/)

[Pipeline - 렌더 파이프라인 구성 예제](app/source/Pipeline/)

[skybox - 스카이박스 렌더링](app/source/skybox/)

[postProcessing - 포스트 프로세싱](app/source/postProcessing/)

[renderGUI - ImGui UI 렌더링](app/source/renderGUI/)

[shadeReflect - SPIR-V 셰이더 리플렉션](app/source/shadeReflect/)

[GLTF - GLTF 모델 로딩 및 렌더링](app/source/GLTF/)



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
