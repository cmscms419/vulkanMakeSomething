# vulkanMakeSomething 프로젝트 구조

HongLabVulkan 기반의 **Vulkan 그래픽스 학습/실습 프로젝트**입니다.

---

## 최상위 디렉토리

```
vulkanMakeSomething/
├── Project/        ← Visual Studio 솔루션 (VKstudio.sln)
├── resource/       ← 공용 리소스 (텍스처, 3D 모델, 폰트)
├── shader/         ← 셰이더 코드
├── external/       ← 외부 라이브러리
├── include/        ← 헤더
├── exe/            ← 빌드 결과물
└── build/          ← 빌드 중간 산출물
```

---

## 솔루션 내 프로젝트 (VKstudio.sln)

### 공유 라이브러리

| 폴더 | 역할 |
|------|------|
| `common/` | 공통 유틸리티 라이브러리 |
| `basicEngine/` | 1세대 기본 엔진 |
| `basicEngine2/` | 2세대 업데이트 엔진 |
| `component/` | 컴포넌트 시스템 v1 |
| `component2/` | 컴포넌트 시스템 v2 |

---

### content 그룹 (구버전 학습 시리즈)

| 솔루션 번호 | 폴더 | 내용 |
|-------------|------|------|
| 00 | `00.basic/` | Vulkan 초기화 기초 |
| 01 | `triangle/` | 삼각형/사각형 렌더링 |
| 02 | `texture/` | 텍스처 적용 |
| 03 | `textureArray/` | 텍스처 배열 |
| 04 | `skymap/` | 스카이맵 |
| 05 | `Mouse/` | 마우스 입력 |
| 06 | `imgui/` | ImGui UI 통합 |
| 07 | `camera/` | 카메라 시스템 |
| 08 | `3DModelLoad/` | 3D 모델 로딩 (OBJ) |
| 09 | `DescriptorCodeUpdate/` | 디스크립터 셋 리팩토링 |
| 10 | `PBRbasic/` | 기본 PBR 렌더링 |
| 11 | `ImageBasedLightingPBR/` | IBL PBR |
| 12 | `11.PBRModel/` | PBR 3D 모델 |
| 13 | `computerShader/` | 컴퓨트 셰이더 |
| 14 | `14.gltfLoader/` | GLTF 로더 |

---

### constent2 그룹 (신버전 - basicEngine2 기반)

| 솔루션 번호 | 폴더 | 내용 |
|-------------|------|------|
| 00 | `00.renderGUI/` | GUI 렌더링 |
| 03 | `03.Pipeline/` | 렌더 파이프라인 |
| 04 | `04.skybox/` | 스카이박스 |
| 05 | `05.postprocessing/` | 포스트 프로세싱 |
| 06 | `06.GLTF/` | GLTF 로딩 (최신) |

---

## resource/ 주요 파일

```
resource/
├── viking_room.obj / .png   ← OBJ 모델 예제
├── sphere.obj / teapot.obj  ← 기하 도형
├── DamagedHelmet.glb        ← GLB 모델
├── Duck.glb / Fox.glb       ← GLB 모델
├── cubeMap/                 ← 큐브맵 텍스처
├── Texture/                 ← 일반 텍스처들
└── Roboto-Medium.ttf        ← ImGui용 폰트
```
