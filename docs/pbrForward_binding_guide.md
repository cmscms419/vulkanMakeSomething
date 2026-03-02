# pbrForward.frag 기반 가상 바인딩 구현 가이드

`pbrForward.frag` 셰이더의 descriptor set 구조를 기준으로,
계획된 `IShaderResource` / `TextureManager` / `DescriptorSetHander::create()` 시스템을
실제로 적용하면 어떤 흐름이 되는지 단계별로 설명한다.

---

## 셰이더 descriptor set 구조

```glsl
// set=0
layout(set=0, binding=0) uniform SceneDataUBO { ... } sceneData;
layout(set=0, binding=1) uniform OptionsUBO   { ... } options;

// set=1 (bindless)
layout(set=1, binding=0) restrict readonly buffer MaterialBuffer { MaterialUBO materials[]; };
layout(set=1, binding=1) uniform sampler2D materialTextures[512];

// set=2 (IBL)
layout(set=2, binding=0) uniform samplerCube prefilteredMap;
layout(set=2, binding=1) uniform samplerCube irradianceMap;
layout(set=2, binding=2) uniform sampler2D   brdfLUT;

// set=3
layout(set=3, binding=0) uniform sampler2DShadow shadowMap;

// push constant
layout(push_constant) uniform PushConstants {
    mat4     model;
    float    coeffs[15];
    uint     materialIndex;  // ← materialBuffer 내 소재 위치
} pushConstants;
```

---

## 핵심 인덱스 흐름

셰이더 안에서 텍스처를 가져오는 핵심 두 줄:

```glsl
// 1. push constant의 materialIndex로 소재 정보 조회
MaterialUBO material = materialBuffer.materials[pushConstants.materialIndex];

// 2. 소재 안의 텍스처 인덱스로 텍스처 배열 접근
vec4 baseColor = texture(materialTextures[nonuniformEXT(material.baseColorTextureIndex)], fragTexCoord);
```

이 두 인덱스를 C++에서 어떻게 만드는지가 핵심이다.

```
textureManager.add("albedo", &tex)
        ↓ 반환값: GPU 배열 인덱스 (예: 0)
MaterialUBO.baseColorTextureIndex = 0
        ↓ materialBuffer SSBO에 저장
mesh.materialIndex = materialBuffer에 저장된 위치 (예: 2)
        ↓ 드로우 시 push constant로 전달
셰이더: materials[2].baseColorTextureIndex → 0
        ↓
셰이더: materialTextures[0] → DamagedHelmet albedo 텍스처
```

---

## Step 1 — 리소스 준비

### C++ 구조체 정의

셰이더의 `struct MaterialUBO`와 메모리 레이아웃을 맞춘다.

```cpp
struct MaterialUBO {
    glm::vec4 emissiveFactor        = glm::vec4(0.0f);
    glm::vec4 baseColorFactor       = glm::vec4(1.0f);
    float     roughnessFactor       = 1.0f;
    float     transparencyFactor    = 1.0f;
    float     discardAlpha          = 0.1f;
    float     metallicFactor        = 1.0f;
    int       baseColorTextureIndex         = -1; // -1 = 텍스처 없음
    int       emissiveTextureIndex          = -1;
    int       normalTextureIndex            = -1;
    int       opacityTextureIndex           = -1;
    int       metallicRoughnessTextureIndex = -1;
    int       occlusionTextureIndex         = -1;
};

struct PbrPushConstants {
    alignas(16) glm::mat4 model   = glm::mat4(1.0f); // 64 bytes
    alignas(4)  float coeffs[15]  = {};               // 60 bytes
    alignas(4)  uint32_t materialIndex = 0;           // 4 bytes
    // 합계: 128 bytes
};
```

### 리소스 객체 생성

```cpp
VKUniformBuffer2<SceneDataUBO>   sceneUBO(ctx);
VKUniformBuffer2<OptionsUniform> optionsUBO(ctx);
VKStorageBuffer                  materialBuffer(ctx);  // SSBO
TextureManager                   textureManager(ctx);  // IShaderResource 구현체

VKImage2D prefilteredMap(ctx);  // IBL
VKImage2D irradianceMap(ctx);
VKImage2D brdfLUT(ctx);
VKImage2D shadowDepth(ctx);
```

---

## Step 2 — 모델 로드 시 텍스처 등록

모델을 불러오면서 각 텍스처를 `TextureManager`에 등록한다.
`add()`의 **반환값이 GPU 배열 내 인덱스**이고, 이 값을 `MaterialUBO`에 저장한다.

```cpp
std::vector<MaterialUBO> materialList;  // 나중에 SSBO에 업로드

for (size_t i = 0; i < model.meshes.size(); ++i) {
    MaterialUBO mat{};
    std::string prefix = "mesh" + std::to_string(i) + "_";

    // textureManager.add() 반환값 = GPU 배열 인덱스
    if (model.hasTexture(i, ALBEDO)) {
        mat.baseColorTextureIndex =
            (int)textureManager.add(prefix + "albedo", &model.texture(i, ALBEDO));
    }
    if (model.hasTexture(i, NORMAL)) {
        mat.normalTextureIndex =
            (int)textureManager.add(prefix + "normal", &model.texture(i, NORMAL));
    }
    if (model.hasTexture(i, METALLIC_ROUGHNESS)) {
        mat.metallicRoughnessTextureIndex =
            (int)textureManager.add(prefix + "metallicRoughness",
                                    &model.texture(i, METALLIC_ROUGHNESS));
    }
    if (model.hasTexture(i, OCCLUSION)) {
        mat.occlusionTextureIndex =
            (int)textureManager.add(prefix + "occlusion", &model.texture(i, OCCLUSION));
    }
    if (model.hasTexture(i, EMISSIVE)) {
        mat.emissiveTextureIndex =
            (int)textureManager.add(prefix + "emissive", &model.texture(i, EMISSIVE));
    }

    mat.baseColorFactor = model.getMaterial(i).baseColorFactor;
    mat.roughnessFactor = model.getMaterial(i).roughnessFactor;
    mat.metallicFactor  = model.getMaterial(i).metallicFactor;

    // materialList 내 위치 = 이 메시의 materialIndex
    model.meshes[i].materialIndex = (uint32_t)materialList.size();
    materialList.push_back(mat);
}
```

### DamagedHelmet 예시 결과

| textureManager 인덱스 | 이름 | 용도 |
|---|---|---|
| 0 | `mesh0_albedo` | `mat.baseColorTextureIndex = 0` |
| 1 | `mesh0_normal` | `mat.normalTextureIndex = 1` |
| 2 | `mesh0_metallicRoughness` | `mat.metallicRoughnessTextureIndex = 2` |
| 3 | `mesh0_occlusion` | `mat.occlusionTextureIndex = 3` |
| 4 | `mesh0_emissive` | `mat.emissiveTextureIndex = 4` |

`mesh.materialIndex = 0` (materialList 첫 번째 항목)

---

## Step 3 — SSBO에 소재 데이터 업로드

```cpp
// 모든 모델 로드 완료 후 한 번에 업로드
materialBuffer.createStorageBuffer(
    sizeof(MaterialUBO) * materialList.size(),
    materialList.data());
```

---

## Step 4 — Descriptor Set 생성

`DescriptorSetHander::create()` 에 **순서대로** `IShaderResource`를 전달한다.
순서 = binding 번호이며, `stageFlags`는 `VKShaderManager`가 SPIRV-Reflect로 자동 주입한다.

```cpp
DescriptorSetHander set0, set1, set2, set3;

// set=0: SceneDataUBO(binding=0) + OptionsUBO(binding=1)
set0.create(ctx, {
    std::ref(sceneUBO),    // binding=0 → stageFlags: VERTEX | FRAGMENT (자동)
    std::ref(optionsUBO),  // binding=1 → stageFlags: FRAGMENT (자동)
});

// set=1: MaterialBuffer(binding=0) + TextureManager(binding=1)
//
// TextureManager::updateBinding() → descriptorCount = MAX_TEXTURES(512)  [layout 상한]
// TextureManager::updateWrite()   → descriptorCount = textures.size()    [실제 등록 수]
set1.create(ctx, {
    std::ref(materialBuffer),  // binding=0: VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    std::ref(textureManager),  // binding=1: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER x512
});

// set=2: IBL 3종
set2.create(ctx, {
    std::ref(prefilteredMap),  // binding=0: samplerCube
    std::ref(irradianceMap),   // binding=1: samplerCube
    std::ref(brdfLUT),         // binding=2: sampler2D
});

// set=3: 그림자 맵
set3.create(ctx, {
    std::ref(shadowDepth),     // binding=0: sampler2DShadow
});
```

### TextureManager 내부 동작

```
updateBinding() 호출 시:
  binding.descriptorType  = COMBINED_IMAGE_SAMPLER
  binding.descriptorCount = 512          ← layout 선언 상한

updateWrite() 호출 시:
  write.descriptorCount   = 5            ← 실제 등록된 DamagedHelmet 텍스처 수
  write.pImageInfo        = imageInfos[] ← 5개 텍스처의 imageInfo 배열

GPU: 512슬롯 layout, 5슬롯만 실제 데이터 기록
     나머지 507슬롯은 PARTIALLY_BOUND 플래그로 비워둠
```

---

## Step 5 — 드로우 루프

```cpp
// ── 프레임당 1번만 바인딩 ──────────────────────────────────────────────────
VkDescriptorSet sets[] = { set0.handle(), set1.handle(), set2.handle(), set3.handle() };
vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipelineLayout, 0, 4, sets, 0, nullptr);

// ── 메시별 드로우 ──────────────────────────────────────────────────────────
for (const auto& model : models) {
    for (const auto& mesh : model.meshes) {

        PbrPushConstants pc{};
        pc.model         = mesh.transform;
        pc.materialIndex = mesh.materialIndex;  // ← materialBuffer 내 소재 위치

        // coeffs 설정 (셰이더 specularWeight, diffuseWeight 등)
        pc.coeffs[0] = 1.0f;  // specularWeight
        pc.coeffs[1] = 1.0f;  // diffuseWeight
        pc.coeffs[2] = 1.0f;  // emissiveWeight
        pc.coeffs[3] = 0.0f;  // shadowOffset
        pc.coeffs[4] = 1.0f;  // metallicMult
        pc.coeffs[5] = 1.0f;  // roughnessMult

        vkCmdPushConstants(cmd, pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, sizeof(PbrPushConstants), &pc);

        vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer, offsets);
        vkCmdBindIndexBuffer(cmd, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
    }
}
```

---

## Step 6 — 셰이더에서의 접근 (자동으로 올바른 텍스처 사용)

```glsl
// push constant에서 materialIndex(=0) 수신
MaterialUBO material = materialBuffer.materials[pushConstants.materialIndex];
// → material.baseColorTextureIndex = 0

// textureManager에 등록된 0번 텍스처 = DamagedHelmet albedo
vec4 baseColor = texture(
    materialTextures[nonuniformEXT(material.baseColorTextureIndex)],
    fragTexCoord
);
```

---

## 전체 흐름 다이어그램

```
앱 시작 시 (한 번)
─────────────────────────────────────────────────────────────────
모델 로드
  └─ textureManager.add("mesh0_albedo", &tex)  → 인덱스 0
  └─ textureManager.add("mesh0_normal", &tex)  → 인덱스 1
  └─ ...
  └─ MaterialUBO { baseColorTextureIndex=0, normalTextureIndex=1, ... }
  └─ materialList[0] = 위 MaterialUBO
  └─ mesh.materialIndex = 0

materialBuffer.createStorageBuffer(materialList)  → SSBO 생성

set0.create({sceneUBO,      optionsUBO})          → binding 0,1
set1.create({materialBuffer, textureManager})      → binding 0,1  ← 핵심
set2.create({prefilteredMap, irradianceMap, brdfLUT})
set3.create({shadowDepth})

─────────────────────────────────────────────────────────────────
매 프레임
─────────────────────────────────────────────────────────────────
vkCmdBindDescriptorSets(set0, set1, set2, set3)   ← 딱 1번

각 메시 드로우:
  pushConstants.materialIndex = mesh.materialIndex (= 0)
  vkCmdPushConstants(...)
  vkCmdDrawIndexed(...)

─────────────────────────────────────────────────────────────────
셰이더 내부
─────────────────────────────────────────────────────────────────
materialBuffer.materials[0]                        → MaterialUBO 조회
  .baseColorTextureIndex = 0
materialTextures[0]                                → DamagedHelmet albedo
materialTextures[1]                                → DamagedHelmet normal
```

---

## C++ 클래스별 책임 요약

| 클래스 | 역할 |
|---|---|
| `IShaderResource` | `updateBinding()` + `updateWrite()` 공통 인터페이스 |
| `VKUniformBuffer2<T>` | UBO — `updateBinding()`: `UNIFORM_BUFFER`, count=1 |
| `VKStorageBuffer` | SSBO — `updateBinding()`: `STORAGE_BUFFER`, count=1 |
| `VKImage2D` | 단일 텍스처 — `updateBinding()`: `COMBINED_IMAGE_SAMPLER`, count=1 |
| `TextureManager` | bindless 배열 — `updateBinding()`: count=512, `updateWrite()`: 실제 수 |
| `DescriptorSetHander::create()` | 순서대로 받아 layout 탐색 + stageFlags 자동 주입 + vkUpdateDescriptorSets |
