# VKModelLoader 디버깅 가이드

> 분석 파일: `include/engine2/VKModelLoader.h`, `app/engine2/VKModelLoader.cpp`, `app/engine2/VKMesh.cpp`

---

## 1. 발견된 버그 및 문제점

### 1-1. 치명적: 포맷 문자열 불일치

`PRINT_TO_LOGGER`가 printf 스타일(`%s`, `%u`)을 사용하는데, 일부 코드에서 C++ fmt 스타일(`{}`)을 혼용하고 있어 런타임에 UB(Undefined Behavior)가 발생합니다.

| 파일 | 라인 | 문제 | 올바른 형태 |
|------|------|------|-------------|
| VKModelLoader.cpp | 663 | `"Processing {} bones..."` | `"Processing %u bones..."` |
| VKModelLoader.cpp | 1043 | `"Found {} embedded textures..."` | `"Found %u embedded textures..."` |
| VKModelLoader.cpp | 1100 | `"Wrote uncompressed texture {}: {} ({}x{})"` | `"Wrote uncompressed texture %u: %s (%ux%u)"` |
| VKModelLoader.cpp | 1109 | `"...to {} directory"` | `"...to %s directory"` |
| VKModelLoader.cpp | 1216 | `"{:.2f} seconds"` | `"%.2f seconds"` |

**타입 불일치 추가 오류:**

```cpp
// Line 1078: i(uint32_t) 를 %s로 출력 → UB
PRINT_TO_LOGGER("Wrote compressed texture %s: %u ...", i, filename, ...);
//                                         ^^  i는 정수인데 %s 사용

// Line 962: float 좌표를 %u로 출력 → 쓰레기값 출력
PRINT_TO_LOGGER("min(%u, %u, %u)", model.boundingBoxMin.x, ...);
//                   ^^ boundingBoxMin.x 는 float

// Line 224: std::string 을 %s에 직접 전달 → UB (c_str() 필요)
PRINT_TO_LOGGER("Texture filename: %s\n", prefix + shortFilename);
//                                        ^^^^^^^^^^^^^^^^^^^^^^^^ std::string → .c_str() 누락
```

---

### 1-2. 치명적: 캐시 읽기 실패가 모두 조용히 실패

`loadFromCache()`에서 스트림 읽기 실패 시 아무 로그 없이 `return`합니다. 어디서 실패했는지 알 수 없습니다.

```cpp
// 현재 코드 (VKModelLoader.cpp:262 예시)
if (!stream.good() || fileVersion != 1) {
    return; // ← 어디서, 왜 실패했는지 전혀 알 수 없음
}

// 같은 패턴이 반복되는 라인들
// 262, 269, 275, 282, 290, 296, 307, 314, 321, 329, 337, 345, 357, 363, 369, 405
```

`writeToCache()`도 동일한 문제:
```cpp
catch (...) {
    // If any error occurs, silently ignore  ← 에러를 완전히 숨김
}
```

---

### 1-3. 높음: VKMesh 캐시 읽기의 vertex/index reset 버그

`readFromBinaryFileStream()`에서 `vertex`와 `index`를 `reset()`하여 `nullptr`로 만들고, 이후 `createBuffers()`에서 null 역참조로 크래시가 발생합니다.

```cpp
// VKMesh.cpp:199-201 (문제 코드)
bool Mesh::readFromBinaryFileStream(std::ifstream &stream)
{
    // ... 데이터 읽기 ...
    this->vertex.reset();  // ← nullptr로 만듦
    this->index.reset();   // ← nullptr로 만듦
    // ...
}

// 이후 createBuffers() 에서 크래시
this->vertex->createModelVertexBuffer(...);  // nullptr 역참조!
```

**수정:** `createBuffers()` 시작 부분에서 null 체크 후 재생성

```cpp
void Mesh::createBuffers(VKcontext &ctx)
{
    if (!this->vertex)
        this->vertex = std::make_unique<VKBaseBuffer2>(ctx);
    if (!this->index)
        this->index = std::make_unique<VKBaseBuffer2>(ctx);
    // ... 나머지 동일
}
```

---

### 1-4. 중간: 변수 섀도잉 (Variable Shadowing)

`loadFromModelFile()` 내부에서 파라미터 `scene`을 같은 이름의 지역 변수로 덮어씁니다.

```cpp
// VKModelLoader.cpp:29 - 파라미터로 받은 scene
void ModelLoader::loadFromModelFile(const string& modelFilename, bool readBistroObj)
{
    const aiScene* scene = importer.ReadFile(...);  // ← 바깥 scene

    // ...텍스쳐 로딩 내부에서
    const aiScene* scene = importer.GetScene();  // Line 151 ← 같은 이름으로 재선언!
```

---

### 1-5. 중간: Dead Code (도달 불가능한 코드)

`printVerticesAndIndices()`에서 `return` 이후 ~65줄이 절대 실행되지 않습니다.

```cpp
void ModelLoader::printVerticesAndIndices() const
{
    PRINT_TO_LOGGER("Total meshes: %u", model.meshes.size());
    // ...

    return;  // Line 966 ← 여기서 항상 반환

    // Lines 968-1033: 완전히 죽어있는 코드 (절대 실행 안 됨)
    for (size_t meshIdx = 0; meshIdx < model.meshes.size(); ++meshIdx) {
        // ...
    }
}
```

---

### 1-6. 낮음: 인수 개수 불일치

```cpp
// Line 971: 포맷에는 %u 4개, 인수는 5개
PRINT_TO_LOGGER("  Mesh %u: vertices = %u, indices = %u, material = %u",
    meshIdx,
    mesh.vertices.size(),
    mesh.materialIndex,      // ← materialIndex 중복
    mesh.indices.size(),
    mesh.materialIndex);     // ← 5번째 인수 (포맷에 자리 없음)
```

---

## 2. 디버깅을 쉽게 만드는 개선 방안

### 2-1. 캐시 읽기에 단계별 로그 추가

```cpp
// 개선된 loadFromCache 패턴
void ModelLoader::loadFromCache(const string& cacheFilename)
{
    std::ifstream stream(cacheFilename, std::ios::binary);
    if (!stream.is_open()) {
        PRINT_TO_LOGGER("[Cache] Cannot open: %s\n", cacheFilename.c_str());
        return;
    }

    uint32_t fileVersion;
    stream.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
    if (!stream.good()) {
        PRINT_TO_LOGGER("[Cache] Failed to read file version from: %s\n", cacheFilename.c_str());
        return;
    }
    if (fileVersion != 1) {
        PRINT_TO_LOGGER("[Cache] Version mismatch: expected 1, got %u\n", fileVersion);
        return;
    }
    PRINT_TO_LOGGER("[Cache] Version OK (%u)\n", fileVersion);

    // 메시 읽기
    uint32_t meshCount;
    stream.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
    PRINT_TO_LOGGER("[Cache] Loading %u meshes...\n", meshCount);

    for (uint32_t i = 0; i < meshCount; ++i) {
        Mesh& mesh = model.addMesh();
        if (!mesh.readFromBinaryFileStream(stream)) {
            PRINT_TO_LOGGER("[Cache] Failed at mesh %u/%u\n", i, meshCount); // ← 어디서 실패했는지 명확
            return;
        }
        PRINT_TO_LOGGER("[Cache] Mesh %u/%u loaded OK\n", i + 1, meshCount);
    }
    // ...
}
```

---

### 2-2. 캐시 파일 유효성 검증 강화

```cpp
// 현재: 캐시 파일 존재 여부만 확인
bool useCache = false;
if (readBistroObj && filesystem::exists(cachePath)) {
    useCache = true;
}

// 개선: 모델 파일보다 캐시가 최신인지 확인
bool useCache = false;
if (readBistroObj && filesystem::exists(cachePath)) {
    auto modelTime = filesystem::last_write_time(modelPath);
    auto cacheTime = filesystem::last_write_time(cachePath);
    if (cacheTime > modelTime) {
        useCache = true;
        PRINT_TO_LOGGER("[Cache] Cache is newer than model, using cache.\n");
    } else {
        PRINT_TO_LOGGER("[Cache] Model is newer than cache, rebuilding.\n");
    }
}
```

---

### 2-3. 섹션별 로딩 타이밍 추가

```cpp
void ModelLoader::loadFromModelFile(const string& modelFilename, bool readBistroObj)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    auto elapsed = [&t0](const char* label) {
        auto now = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
        PRINT_TO_LOGGER("[Timer] %s: %lldms\n", label, ms);
    };

    // ...
    processAnimations(scene);  elapsed("processAnimations");
    processBones(scene);        elapsed("processBones");
    processNode(...);           elapsed("processNode");
    // ...각 단계가 얼마나 걸리는지 정확히 알 수 있음
}
```

---

### 2-4. 포맷 문자열 일관성 - 매크로 wrapper 활용

프로젝트에서 `PRINT_TO_LOGGER`가 printf 스타일이라면, 모든 위치에서 통일합니다:

```cpp
// 잘못된 사용 (수정 필요)
PRINT_TO_LOGGER("Found {} embedded textures", count);   // fmt 스타일 ❌

// 올바른 사용
PRINT_TO_LOGGER("Found %u embedded textures\n", count); // printf 스타일 ✓

// std::string 전달 시
PRINT_TO_LOGGER("Path: %s\n", (prefix + filename).c_str()); // .c_str() 필수
```

---

### 2-5. `printVerticesAndIndices` 수정 - 실제로 동작하도록

```cpp
void ModelLoader::printVerticesAndIndices() const
{
    PRINT_TO_LOGGER("\n=== Model Debug Info ===\n");
    PRINT_TO_LOGGER("  Meshes:    %zu\n", model.meshes.size());
    PRINT_TO_LOGGER("  Materials: %zu\n", model.materials.size());
    PRINT_TO_LOGGER("  BBox min:  (%.2f, %.2f, %.2f)\n",
        model.boundingBoxMin.x, model.boundingBoxMin.y, model.boundingBoxMin.z);
    PRINT_TO_LOGGER("  BBox max:  (%.2f, %.2f, %.2f)\n",
        model.boundingBoxMax.x, model.boundingBoxMax.y, model.boundingBoxMax.z);

    // return; ← 이 줄을 제거하거나, 아래 상세 출력을 별도 함수로 분리
    for (size_t i = 0; i < model.meshes.size(); ++i) {
        const Mesh& mesh = model.meshes[i];
        PRINT_TO_LOGGER("  [Mesh %zu] verts=%zu, indices=%zu, mat=%u\n",
            i, mesh.vertices.size(), mesh.indices.size(), mesh.materialIndex);
    }
}
```

---

## 3. 전체 문제 요약표

| 심각도 | 위치 | 문제 | 영향 |
|--------|------|------|------|
| 🔴 치명 | VKModelLoader.cpp:224 | `std::string`을 `%s`에 직접 전달 | UB, 크래시 가능 |
| 🔴 치명 | VKModelLoader.cpp:663,1043,1100... | `{}` fmt 스타일 오용 | UB, 잘못된 출력 |
| 🔴 치명 | VKModelLoader.cpp:1078 | `uint32_t`를 `%s`로 출력 | UB |
| 🔴 치명 | VKModelLoader.cpp:962,974 | `float`을 `%u`로 출력 | 쓰레기값 출력 |
| 🔴 치명 | VKMesh.cpp:199-201 | `vertex/index.reset()` 후 `createBuffers` 크래시 | 프로그램 크래시 |
| 🟠 높음 | VKModelLoader.cpp:254~405 | 캐시 읽기 실패 무음 반환 | 디버깅 불가 |
| 🟠 높음 | VKModelLoader.cpp:532 | `catch(...)` 무음 무시 | 쓰기 실패 탐지 불가 |
| 🟡 중간 | VKModelLoader.cpp:151 | `scene` 변수 섀도잉 | 잠재적 논리 오류 |
| 🟡 중간 | VKModelLoader.cpp:966 | `printVerticesAndIndices` dead code | 디버그 기능 동작 안 함 |
| 🟢 낮음 | VKModelLoader.cpp:971 | 인수 개수 불일치 | 잘못된 출력 |
| 🟢 낮음 | VKModelLoader.cpp:40-52 | 캐시 신선도 미검증 | 구버전 캐시 사용 가능 |

---

## 4. 최우선 수정 순서

1. **`createBuffers`에 null 체크 추가** ([VKMesh.cpp:38](app/engine2/VKMesh.cpp)) — 크래시 방지
2. **포맷 문자열 전수 수정** — `{}` → `%u/%s`, `std::string` → `.c_str()`
3. **캐시 읽기 실패 로그 추가** — 어느 단계에서 실패하는지 파악 가능하게
4. **`printVerticesAndIndices`의 `return` 제거** — 디버그 출력 복원
5. **캐시 파일 신선도 검증** — 구버전 캐시 자동 무효화
