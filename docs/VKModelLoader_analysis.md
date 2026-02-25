# VKModelLoader.cpp 분석

## 개요

`ModelLoader`는 3D 모델 파일을 읽어 `VKModel`에 데이터를 채우는 클래스입니다.
내부적으로 [Assimp](https://assimp-docs.readthedocs.io/) 라이브러리를 사용하며,
바이너리 캐시(.bin), 스켈레탈 애니메이션, 내장 텍스처(Embedded Texture) 처리까지 지원합니다.

---

## 클래스 구조

```
ModelLoader
 ├── VKModel& model          ← 데이터를 채울 대상 모델 (참조)
 ├── Assimp::Importer importer
 └── string directory        ← 모델 파일이 위치한 디렉토리
```

---

## 전체 흐름도

```
loadFromModelFile()
 │
 ├─[캐시 파일 존재?]──YES──► loadFromCache()
 │                              └─[캐시 로드 성공?]──YES──► 텍스처 로드 후 종료
 │                                                  └──NO ──► model.cleanup() → 아래로 계속
 │
 └─[Assimp로 파일 읽기]
      │
      ├── processMaterial()  또는  processMaterialBistro()   ← 머티리얼 먼저
      │
      ├── processAnimations()   ← 애니메이션/본 매핑 먼저 구축
      ├── processBones()
      │
      ├── processNode()         ← 씬 그래프 순회, 메시 처리
      │    └── processMesh()    ← 정점/인덱스/본 웨이트
      │
      ├── 텍스처 로드
      │    ├── 내장 텍스처(*0, *1...)  → stbi_load_from_memory()
      │    └── 외부 파일 텍스처       → createTextureFromImage()
      │
      └─[readBistroObj && !useCache]
           ├── optimizeMeshesBistro()   ← 메시 병합 최적화
           └── writeToCache()           ← 바이너리 캐시 저장
```

---

## 함수별 상세 분석

### 1. `loadFromModelFile()`

모델 로드의 **진입점**. 두 가지 경로로 분기됩니다.

```
[입력] modelFilename, readBistroObj
  │
  ├─ 캐시 경로 = 모델파일명_cache.bin (같은 디렉토리)
  ├─ readBistroObj == true 이고 캐시 파일이 있으면 → 캐시 로드 시도
  └─ 캐시 없거나 실패하면 → Assimp로 직접 로드
```

**처리 순서가 중요한 이유:**
```
머티리얼 → 애니메이션/본 → 노드/메시 순으로 처리

WHY: processMesh()에서 bone 이름으로 globalBoneIndex를 조회하기 때문에
     반드시 processAnimations()가 먼저 실행되어 본 매핑이 준비되어야 함
```

**Bistro 모델 전용 처리:**
- `readBistroObj = true`이면 Assimp import 옵션을 추가로 설정 (`aiProcess_JoinIdenticalVertices`, `aiProcess_GenSmoothNormals` 등)
- 텍스처 경로 앞에 `LowRes/` 폴더 prefix 추가
- 로드 완료 후 메시 최적화 + 캐시 저장 수행

---

### 2. `loadFromCache()` / `writeToCache()`

성능 최적화를 위한 **바이너리 캐시** 시스템.

**캐시 파일 포맷 (바이너리, 순서대로):**

```
[4B] fileVersion (= 1)
[4B] dirLength
[nB] directory 문자열

[64B] globalInverseTransform (mat4)
[12B] boundingBoxMin (vec3)
[12B] boundingBoxMax (vec3)

[4B] textureCount
 └── 반복:
      [4B] filenameLength
      [nB] filename 문자열
      [1B] sRGB 여부

[4B] meshCount
 └── 반복: mesh.writeToBinaryFileStream()

[4B] materialCount
 └── 반복:
      [4B] materialVersion (= 1)
      [4B] nameLength
      [nB] name 문자열
      머티리얼 속성들 (emissiveFactor, baseColorFactor, roughness 등)
      텍스처 인덱스들 (baseColor, emissive, normal, opacity 등)
      [4B] flags
```

> **주의:** 텍스처 자체(Vulkan 리소스)는 캐시에 저장하지 않음.
> 캐시 로드 후에 반드시 별도로 텍스처 파일을 다시 읽어야 함.

---

### 3. `processNode()` - 씬 그래프 순회

Assimp의 씬은 **트리 구조**입니다. 이 함수가 재귀적으로 순회합니다.

```
aiNode (Assimp 트리)          VKModelNode (내부 트리)
─────────────────────         ──────────────────────
mName          →              name
mTransformation →             localMatrix, translation, rotation, scale
mMeshes[]      →              meshIndices[]  (processMesh 호출)
mChildren[]    →              children[]     (재귀 호출)
```

```cpp
// 루트 노드이면 model.rootNode에 저장
// 자식 노드이면 parent->children에 추가
// 이후 자식들을 재귀 처리
```

---

### 4. `processMesh()` - 메시 데이터 처리

각 `aiMesh`로부터 정점/인덱스/본 웨이트를 `Mesh` 구조체에 채웁니다.

**정점(Vertex2) 처리:**

| 속성 | Assimp 소스 | 없을 때 기본값 |
|------|------------|--------------|
| pos | mVertices[i] | - |
| normal | mNormals[i] | (0, 1, 0) |
| texCoord | mTextureCoords[0][i] | (0, 0) |
| inTangent | mTangents[i] | (1, 0, 0) |
| Bitangent | mBitangents[i] | (0, 0, 1) |

> **Y 좌표 반전:** `texCoord.y = 1.0f - texCoord.y`
> Vulkan은 UV 원점이 좌상단이므로 Y를 뒤집음

**인덱스 처리:**
- 삼각형이 아닌 폴리곤(`mNumIndices != 3`)은 건너뜀

**본 웨이트 처리 (2패스):**
```
1패스: bone 이름 → VKAnimation에서 globalBoneIndex 조회
       → 정점에 (globalBoneIndex, weight) 추가
       (본 이름을 못 찾으면 로컬 인덱스를 fallback으로 사용)

2패스: 각 정점의 본 웨이트 정규화 (normalizeBoneWeights)

WHY 전역 인덱스: 여러 메시에 걸쳐 본이 분산되어 있어도
                 애니메이션 시스템이 하나의 통합 본 배열을 사용하기 때문
```

---

### 5. `processMaterial()` - 일반 PBR 머티리얼

FBX, OBJ, GLTF 등 일반 모델에 사용됩니다.

```
Assimp 속성                    →   VKMaterial.ubo 필드
─────────────────────────────────────────────────────
AI_MATKEY_COLOR_DIFFUSE        →   baseColorFactor
AI_MATKEY_METALLIC_FACTOR      →   metallicFactor
AI_MATKEY_ROUGHNESS_FACTOR     →   roughness
AI_MATKEY_COLOR_EMISSIVE       →   emissiveFactor

aiTextureType_DIFFUSE          →   baseColorTextureIndex    (sRGB=true)
aiTextureType_GLTF_METALLIC_ROUGHNESS → metallicRoughnessTextureIndex
aiTextureType_SPECULAR         →   metallicRoughnessTextureIndex (fallback)
aiTextureType_NORMALS          →   normalTextureIndex
aiTextureType_LIGHTMAP         →   occlusionTextureIndex
aiTextureType_EMISSIVE         →   emissiveTextureIndex
```

**텍스처 중복 방지:**
```cpp
// getTextureIndex 람다:
// 이미 같은 이름의 텍스처가 있으면 기존 인덱스 반환
// 없으면 textureFilenames에 추가 후 새 인덱스 반환
```

---

### 6. `processMaterialBistro()` - Bistro 전용 머티리얼

Amazon Lumberyard Bistro 씬을 위한 특수 처리입니다.

**일반 머티리얼과의 차이점:**

| 항목 | 일반 | Bistro |
|------|------|--------|
| Ambient → Emissive | X | O (AI_MATKEY_COLOR_AMBIENT) |
| 투명도 처리 | 없음 | Opacity, COLOR_TRANSPARENT 키 처리 |
| 텍스처 경로 | 그대로 | `"dummy/"` prefix로 정규화 처리 |
| 재질별 하드코딩 | 없음 | 유리, 금속 등 material name 기반 override |

**하드코딩된 재질 예시:**
```
"MASTER_Glass_Clean"    → transparencyFactor=0.2, discardAlpha=0.75, sTransparent
"MASTER_Frosted_Glass"  → transparencyFactor=0.2, discardAlpha=0.75, sTransparent
"Streetlight_Glass"     → transparencyFactor=0.15, baseColorTexture 제거
"Metal"                 → metallicFactor=1.0, roughness=0.1
```

---

### 7. `processAnimations()` / `processBones()`

```
processAnimations():
  VKAnimation::loadFromScene(scene) 호출
  → 내부적으로 본 이름 → 전역 인덱스 매핑 구축
  → 애니메이션 클립 (키프레임) 로드

processBones():
  씬 전체 메시에서 HasBones() 확인
  총 본 개수 집계 및 로그 출력 (실질적 처리는 processAnimations에서)
```

---

### 8. `optimizeMeshesBistro()` - Bistro 메시 병합

같은 머티리얼을 가진 여러 메시를 하나로 합쳐 **드로우 콜을 줄이는** 최적화.

**병합 대상 머티리얼:**
- `Foliage_Linde_Tree_Large_Orange_Leaves`
- `Foliage_Linde_Tree_Large_Green_Leaves`
- `Foliage_Linde_Tree_Large_Trunk`

```
병합 과정:
1. 같은 머티리얼명을 가진 메시 목록 수집
2. 첫 번째 메시에 나머지 메시의 vertices 이어붙이기
3. 나머지 메시의 indices를 baseVertexCount 오프셋 적용 후 이어붙이기
4. 합쳐진 메시들(첫 번째 제외)의 vertices/indices 비우기
5. 빈 메시 제거 (std::move로 압축 후 erase)
```

> `std::move`를 사용하는 이유: `Mesh`는 복사 대입이 불가능하기 때문 (VKBaseBuffer2 상속)

---

### 9. 텍스처 로드 분기

```
textureFilenames를 순회:

 filename[0] == '*'?
  │
  YES (내장 텍스처)
  │  stbi_load_from_memory() 로 디코딩
  │  createTextureFromPixelData()
  │  mHeight == 0 → 압축 포맷 (PNG, JPG 등)
  │  mHeight > 0  → 비압축 RGBA, aiTexel → RGBA8 변환
  │
  NO (외부 파일)
     readBistroObj → "디렉토리/LowRes/파일명"
     일반 모델    → "디렉토리/파일명" (경로에서 파일명만 추출)
     createTextureFromImage()
```

---

### 10. `debugWriteEmbeddedTextures()`

내장 텍스처를 `debug_textures/` 폴더에 파일로 저장하는 **디버그 전용 유틸리티**.
- 압축 포맷(PNG, JPG): 바이트 헤더(`0x89 0x50...`)로 포맷 자동 감지 후 파일 직접 저장
- 비압축 포맷: `aiTexel[]` → `unsigned char[]` 변환 후 `stbi_write_png()`로 저장

---

## 데이터 흐름 요약

```
모델 파일(.fbx/.obj/.gltf)
    │
    │ Assimp::Importer::ReadFile()
    ▼
aiScene
 ├── mMaterials[]  ──► processMaterial()  ──► VKModel::materials[]
 ├── mAnimations[] ──► processAnimations() ─► VKModel::animation
 ├── mMeshes[]     ──► processMesh()      ──► VKModel::meshes[]
 ├── mTextures[]   ──► (내장) stbi_load   ──► VKModel::textures[]
 └── mRootNode     ──► processNode()      ──► VKModel::rootNode (트리)
                                               VKModel::textureFilenames[]
                                               ──► (외부) createTextureFromImage()
                                                   ──► VKModel::textures[]
```

---

## 주의사항 및 알아둘 점

| 항목 | 내용 |
|------|------|
| **처리 순서** | 머티리얼 → 애니메이션/본 → 노드/메시 순서를 반드시 지켜야 함 |
| **캐시 한계** | Vulkan 텍스처 리소스는 직렬화 불가 → 캐시 로드 후 텍스처는 항상 다시 로드 |
| **Bistro 전용** | `processMaterialBistro`, `optimizeMeshesBistro`는 Bistro 씬 전용 코드 |
| **Y축 반전** | UV의 Y좌표를 `1 - y`로 반전 (Vulkan UV 좌표계 대응) |
| **전역 본 인덱스** | 메시 로컬 bone index가 아닌 VKAnimation 시스템의 전역 인덱스를 사용 |
| **이동 연산** | Mesh는 복사 불가(VKBaseBuffer2 상속) → `std::move` 사용 필수 |
