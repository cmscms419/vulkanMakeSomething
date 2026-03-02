# SPIRV-Reflect 기반 Vulkan 엔진 추상화 계획

## 배경 (Context)

현재 엔진은 SPIRV-Reflect를 이미 사용하고 있지만(`VKShaderManager`에서 descriptor layout 생성 용도) 두 가지 핵심 기능이 없다:

1. **셰이더 리소스 추적**: 각 파이프라인이 어떤 descriptor set / binding / push constant / vertex input을 요구하는지를 런타임에 이름과 함께 조회할 방법이 없음 (현재는 내부적으로만 처리됨)
2. **Render Graph**: 각 렌더링 패스(shadow, forward, post 등)가 `VKforwardRenderer::draw()`에 하드코딩되어 있고, 배리어도 inline으로 직접 삽입됨. 패스 추가/제거가 어려움.

---

## 목표 1: ShaderResourceLayout — 셰이더 리소스 자동 추적

### 새 파일: `include/engine2/ShaderResourceLayout.h`

```cpp
namespace vkengine {

struct BindingDesc {
    uint32_t           set;
    uint32_t           binding;
    VkDescriptorType   type;        // UBO, SSBO, CombinedSampler, etc.
    uint32_t           count;       // 배열 크기 (보통 1)
    VkShaderStageFlags stageFlags;  // 어느 셰이더 스테이지에서 사용하는지
    std::string        name;        // SPIR-V 바인딩 이름 (e.g. "uScene")
};

struct PushConstantDesc {
    VkShaderStageFlags stageFlags;
    uint32_t           offset;
    uint32_t           size;
    std::string        name;
};

struct VertexInputDesc {
    uint32_t    location;
    VkFormat    format;
    uint32_t    offset;   // vertex struct 내 바이트 오프셋
    std::string name;     // e.g. "inPosition"
};

struct DescriptorSetLayout {
    uint32_t                 setIndex;
    std::vector<BindingDesc> bindings; // binding 번호 순 정렬
};

struct ShaderResourceLayout {
    std::string                       pipelineName;
    std::vector<DescriptorSetLayout>  sets;          // setIndex 순 정렬
    std::optional<PushConstantDesc>   pushConstant;  // 없으면 nullopt
    std::vector<VertexInputDesc>      vertexInputs;  // compute면 비어있음

    const BindingDesc* findBinding(uint32_t set, uint32_t binding) const;
    const BindingDesc* findBindingByName(const std::string& name) const;
};

} // namespace vkengine
```

### VKShaderManager 확장 (additive only)

**`include/engine2/VKShaderManager.h` 추가 항목:**

```cpp
// public:
const ShaderResourceLayout& getResourceLayout(cString pipelineName) const;
void printAllResourceLayouts() const;

// private:
std::unordered_map<cString, ShaderResourceLayout> resourceLayouts;
void buildResourceLayouts();  // constructor 마지막에 호출
```

**`buildResourceLayouts()` 알고리즘** (`app/engine2/VKShaderManager.cpp`):

`pipelineShaders` map을 순회하며 각 파이프라인에 대해:
- 모든 셰이더의 `reflectModule.descriptor_bindings` → `BindingDesc` 수집 (set/binding 기준 병합, stageFlags |=)
- `reflectModule.push_constant_blocks[0]` → `PushConstantDesc`
- vertex 셰이더의 `input_variables` → `VertexInputDesc` (기존 `makeVertexInputAttributeDescriptions()` 로직 재사용)
- 결과를 `resourceLayouts[pipelineName]`에 저장

> **핵심**: 기존 `collectPerPipelineBindings()`가 Vulkan layout 생성용으로 이미 동일한 루프를 수행 중.
> `buildResourceLayouts()`는 같은 reflect 데이터를 **이름과 함께 별도 저장**하는 것.

### VKPipeLineHandle 확장

```cpp
// 추가 (shaderManager에 위임)
const ShaderResourceLayout& getResourceLayout() const;
```

---

## 목표 2: RenderGraph — 파이프라인 그래프 구조

### 설계 원칙

- `VKforwardRenderer`는 **그대로 유지** (비파괴적 변경)
- RenderGraph가 그 **위에서** 패스 실행 순서와 배리어를 관리
- 기존 `VKBarrierHelper::transitionImageLayout2()` 재사용
- 패스 추가 = 그래프에 노드 하나 추가, 렌더러 코드 수정 불필요

### 새 파일: `include/engine2/RenderGraph.h`

```cpp
namespace vkengine {

using ResourceHandle = std::string; // "forwardColor", "shadowDepth", "swapchain"

enum class ResourceAccess {
    ColorAttachmentWrite,
    DepthAttachmentWrite,
    ShaderReadOnly,
    ShaderReadWrite,  // compute general
    Present,
};

struct ResourceUsage {
    ResourceHandle handle;
    ResourceAccess access;
};

struct ResourceEntry {
    VkImage          image       = VK_NULL_HANDLE;
    VkImageView      imageView   = VK_NULL_HANDLE;
    VKBarrierHelper* barrier     = nullptr; // non-owning
    VkFormat         format      = VK_FORMAT_UNDEFINED;
    bool             isSwapchain = false;
};

using PassExecuteFunc = std::function<void(VkCommandBuffer, uint32_t frameIndex)>;

struct RenderPassNode {
    std::string                name;
    std::vector<ResourceUsage> inputs;   // 이 패스가 읽는 리소스
    std::vector<ResourceUsage> outputs;  // 이 패스가 쓰는 리소스
    PassExecuteFunc            execute;
};

class RenderGraph {
public:
    explicit RenderGraph(VKcontext& ctx);

    void registerResource(const ResourceHandle& h, VKImage2D& img);
    void registerSwapchainResource(const ResourceHandle& h, VkFormat fmt);
    void addPass(RenderPassNode pass);

    bool compile();  // addPass 후 한 번만 호출 (위상 정렬 + 유효성 검사)

    void execute(VkCommandBuffer cmd, uint32_t frameIndex,
                 VkImage swapchainImage, VkImageView swapchainView);

    void printGraph() const;

private:
    VKcontext& ctx;
    std::unordered_map<ResourceHandle, ResourceEntry> resources;
    ResourceHandle swapchainHandle;
    std::vector<RenderPassNode> passes;
    std::vector<size_t> sortedOrder; // compile()이 채워넣음

    std::vector<std::vector<size_t>> buildAdjacency() const;
    std::vector<size_t> topologicalSort(const std::vector<std::vector<size_t>>& adj) const;
    void insertBarriersBeforePass(VkCommandBuffer cmd, const RenderPassNode& pass);

    struct BarrierParams {
        VkImageLayout         layout;
        VkAccessFlags2        access;
        VkPipelineStageFlags2 stage;
    };
    static BarrierParams toBarrierParams(ResourceAccess access, VkFormat format);
};

} // namespace vkengine
```

### 핵심 알고리즘

#### 위상 정렬 (Kahn's BFS)

```
writers[handle] = {패스 인덱스들}       // 각 리소스를 쓰는 패스
adj[i] → j  if 패스i.output이 패스j.input에 존재
inDegree 계산 → BFS → sortedOrder
사이클 감지: result.size() != passes.size() → 오류
```

#### 배리어 삽입 (패스 실행 직전)

```
for each (input + output) of pass:
    newParams = toBarrierParams(access, format)
    if barrier->Currentlayout() == newParams.layout: skip (no-op)
    else: VkImageMemoryBarrier2 생성
vkCmdPipelineBarrier2() 한 번에 배치 발행
barrier->Currentlayout/access/stage 갱신
```

#### ResourceAccess → Vulkan 매핑

| Access | Layout | AccessFlags2 | StageFlags2 |
|--------|--------|--------------|-------------|
| ColorAttachmentWrite | COLOR_ATTACHMENT_OPTIMAL | COLOR_ATTACHMENT_WRITE | COLOR_ATTACHMENT_OUTPUT |
| DepthAttachmentWrite | DEPTH_STENCIL_ATTACHMENT_OPTIMAL | DEPTH_STENCIL_ATTACHMENT_WRITE | EARLY_FRAGMENT_TESTS |
| ShaderReadOnly | SHADER_READ_ONLY_OPTIMAL | SHADER_READ | FRAGMENT_SHADER |
| ShaderReadWrite | GENERAL | SHADER_READ \| SHADER_WRITE | COMPUTE_SHADER |
| Present | PRESENT_SRC_KHR | NONE | BOTTOM_OF_PIPE |

### 사용 예시

```cpp
// Application2 또는 VKengine2에서 초기화 시:
RenderGraph graph(ctx);

graph.registerResource("shadowDepth",  renderer.getShadowDepthImage());
graph.registerResource("forwardColor", renderer.getForwardColorImage());
graph.registerSwapchainResource("swapchain", swapchainFormat);

// 패스 등록 (등록 순서 무관, compile()이 자동 정렬)
graph.addPass({
    .name    = "shadow",
    .inputs  = {},
    .outputs = {{"shadowDepth", ResourceAccess::DepthAttachmentWrite}},
    .execute = [&](VkCommandBuffer cmd, uint32_t frame) {
        renderer.makeShadowMap(cmd, frame, models);
    }
});

graph.addPass({
    .name    = "forward",
    .inputs  = {{"shadowDepth",  ResourceAccess::ShaderReadOnly}},
    .outputs = {{"forwardColor", ResourceAccess::ColorAttachmentWrite}},
    .execute = [&](VkCommandBuffer cmd, uint32_t frame) {
        renderer.drawForward(cmd, frame, models, viewport, scissor);
    }
});

graph.addPass({
    .name    = "post",
    .inputs  = {{"forwardColor", ResourceAccess::ShaderReadOnly}},
    .outputs = {{"swapchain",    ResourceAccess::ColorAttachmentWrite}},
    .execute = [&](VkCommandBuffer cmd, uint32_t frame) {
        renderer.drawPost(cmd, frame, swapchainView, viewport, scissor);
    }
});

graph.compile();  // 한 번만

// 매 프레임:
graph.execute(cmd, frameIndex, swapchainImage, swapchainView);
```

---

## 목표 3: IShaderResource — 상속 기반 셰이더 리소스 바인딩

> **HonglabVulkan 예시 참고**: `Resource` (추상 기반) → `Image2D`, `MappedBuffer`, `TextureManager` 구조를 그대로 채용.
> 예시의 `Resource::updateBinding()` + `Resource::updateWrite()` 패턴이 핵심이다.

### 현재 구조

`VKBaseBuffer2`와 `VKImage2D`는 모두 `VKResourceBinding`을 멤버로 가지지만 공통 인터페이스가 없다:

```
VKBaseBuffer2  →  (has)  VKResourceBinding  →  (has) VKBarrierHelper
VKImage2D      →  (has)  VKResourceBinding  →  (has) VKBarrierHelper
```

### 새 파일: `include/engine2/IShaderResource.h`

예시의 `Resource`와 동일한 패턴 — `updateBinding()` / `updateWrite()` 두 메서드로 역할 분리:

```cpp
namespace vkengine {

class IShaderResource {
public:
    virtual ~IShaderResource() = default;

    // [1] Descriptor Set Layout 정보 — descriptorType, descriptorCount 기입
    //     stageFlags는 ShaderManager가 SPIRV-Reflect로 결정해서 주입함
    virtual void updateBinding(VkDescriptorSetLayoutBinding& binding) = 0;

    // [2] 실제 GPU 리소스 데이터 — bufferInfo 또는 imageInfo 채우기
    //     dstSet / dstBinding 은 DescriptorSetHander::create() 가 덮어씀
    virtual void updateWrite(VkWriteDescriptorSet& write) = 0;

    // [3] BarrierHelper 접근 (RenderGraph 배리어 삽입용)
    virtual VKResourceBinding& getResourceBinding() = 0;
    virtual const VKResourceBinding& getResourceBinding() const = 0;
};

} // namespace vkengine
```

### VKBaseBuffer2 override

```cpp
// include/engine2/VKbuffer2.h
class VKBaseBuffer2 : public IShaderResource {
public:
    void updateBinding(VkDescriptorSetLayoutBinding& binding) override {
        binding.descriptorType  = resourceBinding.descriptorType; // UBO / SSBO
        binding.descriptorCount = 1;
        binding.stageFlags      = 0; // ShaderManager가 stageFlags 주입
    }

    void updateWrite(VkWriteDescriptorSet& write) override {
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType  = resourceBinding.descriptorType;
        write.descriptorCount = 1;
        write.pBufferInfo     = &resourceBinding.bufferInfo;
    }

    VKResourceBinding& getResourceBinding() override       { return resourceBinding; }
    const VKResourceBinding& getResourceBinding() const override { return resourceBinding; }
};
```

### VKImage2D override

```cpp
// include/engine2/VKImage2D.h
class VKImage2D : public IShaderResource {
public:
    void updateBinding(VkDescriptorSetLayoutBinding& binding) override {
        binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags      = 0;
    }

    void updateWrite(VkWriteDescriptorSet& write) override {
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo      = &resourceBinding.imageInfo;
    }

    VKResourceBinding& getResourceBinding() override       { return resourceBinding; }
    const VKResourceBinding& getResourceBinding() const override { return resourceBinding; }
};
```

### DescriptorSetHander::create() — 순서 기반 자동 바인딩

> 예시의 `DescriptorSet::create(vector<reference_wrapper<Resource>>)` 패턴 채용.
> **이름 매핑 불필요** — 리소스를 셰이더 binding 순서대로 전달하면 자동으로 처리됨.

`include/engine2/VKDescriptorSet.h` 또는 `DescriptorSetHander` 에 `create()` 오버로드 추가:

```cpp
void create(VKcontext& ctx,
            const std::vector<std::reference_wrapper<IShaderResource>>& resources)
{
    // 1. 각 리소스의 updateBinding()으로 layout bindings 구성
    //    binding 번호 = vector 내 순서 인덱스
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(resources.size());
    for (size_t i = 0; i < resources.size(); i++) {
        resources[i].get().updateBinding(layoutBindings[i]);
        layoutBindings[i].binding = uint32_t(i);
    }

    // 2. VKDescriptorManager2에서 매칭 layout 찾기 (SPIRV-Reflect stageFlags 포함)
    VkDescriptorSetLayout layout = ctx.descriptorPool().descriptorSetLayout(layoutBindings);
    // 완전한 layoutBindings (stageFlags 채워진 버전) 재취득
    layoutBindings = ctx.descriptorPool().layoutToBindings(layout);

    // 3. descriptor set 할당
    descriptorSet = ctx.descriptorPool().allocateDescriptorSet(layout);

    // 4. 각 리소스의 updateWrite()로 write 구성 후 일괄 업데이트
    std::vector<VkWriteDescriptorSet> writes(resources.size());
    for (size_t i = 0; i < resources.size(); i++) {
        resources[i].get().updateWrite(writes[i]);
        writes[i].dstSet     = descriptorSet;      // ← DescriptorSetHander가 덮어씀
        writes[i].dstBinding = layoutBindings[i].binding;
    }
    vkUpdateDescriptorSets(device, uint32_t(writes.size()), writes.data(), 0, nullptr);
}
```

### 사용 예시 — ShaderResourceBinder 없이도 동작

```cpp
// 리소스를 셰이더 binding 순서대로 나열 (set=0: binding 0,1,2...)
DescriptorSetHander sceneSet;
sceneSet.create(ctx, {
    std::ref(sceneUBO),   // binding=0: uScene
    std::ref(skyUBO),     // binding=1: uSkyOptions
    std::ref(optionsUBO), // binding=2: uOptions
});

// 바인딩 완료 — stageFlags는 ShaderManager SPIRV-Reflect 결과에서 자동 주입됨
```

---

## 구현 순서

### Phase 1: ShaderResourceLayout (렌더 그래프와 독립적)

1. `include/engine2/ShaderResourceLayout.h` 생성 — 구조체 정의
2. `app/engine2/ShaderResourceLayout.cpp` 생성 — `findBinding()`, `findBindingByName()` 구현
3. `include/engine2/VKShaderManager.h` 수정 — `resourceLayouts` map, `buildResourceLayouts()`, `getResourceLayout()` 선언
4. `app/engine2/VKShaderManager.cpp` 수정 — `buildResourceLayouts()` 구현, constructor 마지막에 호출
5. `include/engine2/VKpipeLineHandle.h` / `.cpp` 수정 — `getResourceLayout()` 추가
6. **검증**: 엔진 시작 시 `printAllResourceLayouts()` 호출 → 콘솔 출력 확인

### Phase 2: IShaderResource + DescriptorSetHander::create() (Phase 1 이후)

7. `include/engine2/IShaderResource.h` 생성 — `updateBinding()`, `updateWrite()`, `getResourceBinding()` 순수 가상 선언
8. `include/engine2/VKbuffer2.h` 수정 — `IShaderResource` 상속, 3개 메서드 override
9. `include/engine2/VKImage2D.h` 수정 — `IShaderResource` 상속, 3개 메서드 override
10. `include/engine2/VKDescriptorSet.h` 수정 — `create(vector<reference_wrapper<IShaderResource>>)` 오버로드 추가
11. **검증**: 기존 `DescriptorSetHander` 생성 코드를 새 `create()` 로 교체, 컴파일 + 시각적 결과 동일 확인

### Phase 3: RenderGraph (Phase 1 이후, Phase 2와 병행 가능)

13. `include/engine2/RenderGraph.h` 생성 — 클래스 정의
14. `app/engine2/RenderGraph.cpp` 생성 — `buildAdjacency()`, `topologicalSort()`, `compile()`, `execute()`, `insertBarriersBeforePass()`, `toBarrierParams()` 구현
15. `include/component2/VKrenderer.h` / `.cpp` 수정 — `getShadowDepthImage()`, `getForwardColorImage()` getter 추가
16. `Application2` 또는 `VKengine2` 수정 — 기존 `draw()` / `makeShadowMap()` 직접 호출을 `graph.execute()`로 교체
17. **검증**: 시각적 결과가 기존과 동일한지 확인, `graph.printGraph()`로 노드/엣지 출력 확인

---

## 수정 파일 요약

### 새 파일 생성 (Phase 1~3)

| 파일 | 내용 |
|------|------|
| `include/engine2/IShaderResource.h` | `updateBinding()`, `updateWrite()`, `getResourceBinding()` 순수 가상 |
| `include/engine2/ShaderResourceLayout.h` | `BindingDesc`, `PushConstantDesc`, `VertexInputDesc`, `ShaderResourceLayout` |
| `app/engine2/ShaderResourceLayout.cpp` | `findBinding()`, `findBindingByName()` |
| `include/engine2/RenderGraph.h` | `ResourceAccess`, `RenderPassNode`, `RenderGraph` |
| `app/engine2/RenderGraph.cpp` | 위상 정렬, 배리어 삽입, 실행 루프 |

### 기존 파일 수정 (additive, Phase 1~3)

| 파일 | 변경 내용 |
|------|-----------|
| `include/engine2/VKbuffer2.h` | `IShaderResource` 상속, `updateBinding()` / `updateWrite()` / `getResourceBinding()` override |
| `include/engine2/VKImage2D.h` | `IShaderResource` 상속, 위 3개 메서드 override |
| `include/engine2/VKDescriptorSet.h` | `create(vector<reference_wrapper<IShaderResource>>)` 오버로드 추가 |
| `include/engine2/VKShaderManager.h` | `resourceLayouts`, `buildResourceLayouts()`, `getResourceLayout()` 추가 |
| `app/engine2/VKShaderManager.cpp` | `buildResourceLayouts()` 구현, constructor에 호출 추가 |
| `include/engine2/VKpipeLineHandle.h` | `getResourceLayout()` 선언 |
| `app/engine2/VKpipeLineHandle.cpp` | `getResourceLayout()` 구현 |
| `include/component2/VKrenderer.h` | 이미지 getter 메서드 선언 |
| `app/component2/VKrenderer.cpp` | 이미지 getter 구현 |

---

## 목표 4: BindlessResourceTable — 인덱스 기반 리소스 바인딩

### 현재 방식의 문제

현재는 메시마다 descriptor set을 새로 바인딩한다:

```
DamagedHelmet 드로우: vkCmdBindDescriptorSets(set=1, { albedo, normal, metallicRoughness, ... })
Fox 드로우:           vkCmdBindDescriptorSets(set=1, { fox_albedo, fox_normal, ... })
Rock 드로우:          vkCmdBindDescriptorSets(set=1, { rock_albedo, ... })
→ 모델 100개 = 100번의 vkCmdBindDescriptorSets → CPU 병목
```

### Bindless 방식

모든 리소스를 GPU에 한 번에 올리고, 셰이더에 인덱스만 전달:

```
앱 시작 시 (한 번):
  table[0] = helmetAlbedo    table[1] = helmetNormal
  table[2] = foxAlbedo       table[3] = foxNormal

매 프레임 (딱 1번만 바인딩):
  vkCmdBindDescriptorSets(set=0, bindlessTable)

DamagedHelmet 드로우: vkCmdPushConstants({ albedoIdx=0, normalIdx=1, modelMatrix=... })
Fox 드로우:           vkCmdPushConstants({ albedoIdx=2, normalIdx=3, modelMatrix=... })
→ descriptor 바인딩 0번, push constant만 변경
```

### 필요한 Vulkan 기능 활성화

`VKdeviceHandler2::createLogicalDevice()`에 추가:

```cpp
VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
indexingFeatures.descriptorBindingPartiallyBound              = VK_TRUE; // 빈 슬롯 허용
indexingFeatures.runtimeDescriptorArray                       = VK_TRUE; // 셰이더에서 [] 사용
indexingFeatures.descriptorBindingUpdateAfterBind             = VK_TRUE; // 바인딩 후 업데이트 가능
indexingFeatures.shaderSampledImageArrayNonUniformIndexing    = VK_TRUE;
indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
// pNext 체인에 연결
```

> Vulkan 1.2 코어 기능이므로 현재 엔진(Vulkan 1.3 사용 중)에서 별도 extension 없이 사용 가능.

### 최대 개수 상한 + SPIRV-Reflect 기반 실제 할당 개수

두 가지 개념을 구분해야 한다:

| 개념 | 값 | 역할 |
|------|-----|------|
| `MAX_TEXTURES` | 1024 (상수) | Descriptor Set **Layout** 선언 시 최대 슬롯 수 (절대 상한) |
| `actualCount` | `textures.size()` | Descriptor Set **할당** 시 실제 사용 개수 (SPIRV-Reflect 기반) |

**SPIRV-Reflect 연동 방식**:

```
ShaderResourceLayout의 BindingDesc::count 값:
  count == 0  → runtime 배열 ([]  선언) → MAX_TEXTURES를 상한으로 사용
  count == N  → 고정 크기 배열         → N 을 그대로 사용
```

`textures.size()`는 `registerTexture()` 호출 횟수로 증가 → 실제 등록된 텍스처 수 = SPIRV-Reflect가 필요하다고 파악한 수량에 맞게 호출한 결과.

**핵심 Vulkan 기능 추가 필요**:

```cpp
// VKdeviceHandler2::createLogicalDevice() 추가
indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE; // 가변 할당 허용
```

### 새 파일: `include/engine2/BindlessResourceTable.h`

```cpp
namespace vkengine {

class BindlessResourceTable {
public:
    // 절대 상한 — layout 선언에만 사용
    static constexpr uint32_t MAX_TEXTURES = 1024;
    static constexpr uint32_t MAX_BUFFERS  = 256;

    explicit BindlessResourceTable(VKcontext& ctx);
    void cleanup();

    // IShaderResource를 등록하고 배열 내 인덱스 반환
    // → textures.size()가 실제 할당 크기를 결정
    uint32_t registerTexture(IShaderResource* resource); // non-owning
    uint32_t registerBuffer(IShaderResource* resource);  // non-owning

    // 미할당 상태면 실제 등록 개수로 descriptor set 생성 후 GPU에 반영
    // 이미 할당됐으면 dirtyList만 업데이트
    void commit();

    // SPIRV-Reflect로 산출한 실제 필요 개수를 외부에서 전달하는 버전
    // ShaderResourceLayout에서 count를 읽어 여기에 넘긴다
    void setExpectedTextureCounts(uint32_t texCount, uint32_t bufCount);

    VkDescriptorSet       getDescriptorSet()  const { return bindlessSet; }
    VkDescriptorSetLayout getLayout()         const { return bindlessLayout; }

    uint32_t getRegisteredTextureCount() const { return uint32_t(textures.size()); }
    uint32_t getRegisteredBufferCount()  const { return uint32_t(buffers.size()); }

private:
    VKcontext& ctx;
    VkDescriptorPool      pool           = VK_NULL_HANDLE;
    VkDescriptorSet       bindlessSet    = VK_NULL_HANDLE;
    VkDescriptorSetLayout bindlessLayout = VK_NULL_HANDLE;

    std::vector<IShaderResource*> textures; // index → IShaderResource* (non-owning)
    std::vector<IShaderResource*> buffers;
    std::vector<uint32_t>         dirtyTextures;
    std::vector<uint32_t>         dirtyBuffers;

    // setExpectedTextureCounts()로 설정; 0이면 textures.size() 사용
    uint32_t expectedTexCount = 0;
    uint32_t expectedBufCount = 0;

    void allocateDescriptorSet(); // commit() 내부에서 최초 1회 호출
};

} // namespace vkengine
```

**Descriptor Set Layout 생성** (`BindlessResourceTable` 생성자):

```cpp
// binding=1: textures[] 배열
VkDescriptorSetLayoutBinding texBinding{};
texBinding.binding         = 1;
texBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
texBinding.descriptorCount = MAX_TEXTURES;  // 상한 선언 (실제 할당은 가변)
texBinding.stageFlags      = VK_SHADER_STAGE_ALL;

VkDescriptorBindingFlags texFlags =
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT         // 미기록 슬롯 허용
  | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT        // 바인딩 후 업데이트 가능
  | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT; // ← 할당 시 실제 개수 지정 가능

VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCI{};
flagsCI.bindingCount  = /* 전체 binding 수 */;
flagsCI.pBindingFlags = &texFlags;  // 각 binding마다 플래그 배열
```

**Descriptor Set 할당** (`allocateDescriptorSet()` 내부):

```cpp
// 실제 개수 결정: setExpectedTextureCounts()로 설정됐으면 그 값, 없으면 등록된 수
uint32_t actualTexCount = (expectedTexCount > 0)
    ? std::min(expectedTexCount, MAX_TEXTURES)
    : std::min(uint32_t(textures.size()), MAX_TEXTURES);

// 가변 개수로 할당
VkDescriptorSetVariableDescriptorCountAllocateInfo varCountInfo{};
varCountInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
varCountInfo.descriptorSetCount = 1;
varCountInfo.pDescriptorCounts  = &actualTexCount;  // ← MAX_TEXTURES 아닌 실제 필요한 수

VkDescriptorSetAllocateInfo allocInfo{};
allocInfo.pNext              = &varCountInfo;
allocInfo.descriptorPool     = pool;
allocInfo.descriptorSetCount = 1;
allocInfo.pSetLayouts        = &bindlessLayout;

vkAllocateDescriptorSets(device, &allocInfo, &bindlessSet);
// GPU에는 actualTexCount 개의 슬롯만 확보됨 (메모리 절약)
```

**`commit()` 내부 동작**:
```
if (bindlessSet == VK_NULL_HANDLE):
    allocateDescriptorSet()   ← 등록 완료 후 첫 commit() 시 실제 개수로 할당

for each idx in dirtyTextures:
    textures[idx]->updateWrite(write)   // IShaderResource::updateWrite() 호출
    write.dstSet          = bindlessSet
    write.dstBinding      = 1           // textures[] 슬롯
    write.dstArrayElement = idx         // 배열 내 위치
    writes.push_back(write)

vkUpdateDescriptorSets(device, writes)
dirtyTextures.clear()
```

**`setExpectedTextureCounts()` 활용** — `ShaderResourceLayout`에서 각 파이프라인이 실제로 필요한 개수를 읽어 전달:

```cpp
// 초기화 시:
const auto& layout = shaderManager.getResourceLayout("forward");

// layout에서 textures[] 바인딩 찾기 (count==0 이면 runtime array)
if (auto* b = layout.findBindingByName("textures")) {
    uint32_t needed = (b->count == 0)
        ? bindlessTable.getRegisteredTextureCount()  // 실제 등록된 수
        : b->count;                                  // 셰이더 선언 고정 크기
    bindlessTable.setExpectedTextureCounts(needed, bufNeeded);
}

bindlessTable.commit();  // → actualTexCount = needed 로 GPU 할당
```

### Descriptor Set Layout 구조 변경

```
[기존]
set=0: { uScene(UBO) }
set=1: { texAlbedo, texNormal, texMetallicRoughness, texOcclusion, texEmissive }

[변경 후]
set=0, binding=0: uScene(UBO)               ← Scene 공통 데이터
set=0, binding=1: sampler2D textures[1024]  ← 모든 텍스처 (bindless 배열)
set=0, binding=2: buffer    buffers[256]    ← 모든 UBO/SSBO (bindless 배열)
push_constant:    DrawResourceIndices       ← 드로우별 인덱스
```

### `DrawResourceIndices` 구조체

```cpp
// include/struct/ubo.h에 추가
struct DrawResourceIndices {
    cMat4    modelMatrix;              // 기존 push constant
    uint32_t albedoIdx            = 0; // bindlessTable.textures[] 인덱스
    uint32_t normalIdx            = 0;
    uint32_t metallicRoughnessIdx = 0;
    uint32_t occlusionIdx         = 0;
    uint32_t emissiveIdx          = 0;
    uint32_t materialBufferIdx    = 0; // bindlessTable.buffers[] 인덱스
    uint32_t padding[2];               // 16바이트 정렬
};
// VKPushConstants<DrawResourceIndices> 로 사용
```

### 셰이더 변경 (`shader/pbrForward.frag`)

```glsl
// 기존
layout(set=1, binding=0) uniform sampler2D texAlbedo;
layout(set=1, binding=1) uniform sampler2D texNormal;

// 변경 후
layout(set=0, binding=1) uniform sampler2D textures[];  // bindless 런타임 배열

layout(push_constant) uniform DrawData {
    mat4     modelMatrix;
    uint     albedoIdx;
    uint     normalIdx;
    uint     metallicRoughnessIdx;
    uint     occlusionIdx;
    uint     emissiveIdx;
    uint     materialBufferIdx;
} draw;

// 사용
vec4 albedo = texture(textures[draw.albedoIdx], inUV);
vec3 normal = texture(textures[draw.normalIdx], inUV).rgb;
```

### 사용 흐름

```cpp
// 앱 초기화 시:
BindlessResourceTable bindlessTable(ctx);

// 모델 로드 후 텍스처 등록:
uint32_t helmetAlbedoIdx  = bindlessTable.registerTexture(&helmetAlbedo);  // → 0
uint32_t helmetNormalIdx  = bindlessTable.registerTexture(&helmetNormal);  // → 1
uint32_t foxAlbedoIdx     = bindlessTable.registerTexture(&foxAlbedo);     // → 2

bindlessTable.commit();  // GPU에 반영

// 매 프레임:
vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
    layout, 0, 1, &bindlessTable.getDescriptorSet(), 0, nullptr);
// ↑ 딱 1번만!

// 각 메시 드로우:
DrawResourceIndices drawData{};
drawData.modelMatrix  = mesh.transform;
drawData.albedoIdx    = helmetAlbedoIdx;  // 0
drawData.normalIdx    = helmetNormalIdx;  // 1
vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_ALL, 0, sizeof(drawData), &drawData);
vkCmdDrawIndexed(cmd, mesh.indexCount, ...);
```

---

## Phase 4: BindlessResourceTable 구현 순서 (Phase 2 이후)

18. `app/engine2/VKdeviceHandler2.cpp` 수정 — `VkPhysicalDeviceDescriptorIndexingFeatures` 활성화
19. `include/engine2/BindlessResourceTable.h` 생성
20. `app/engine2/BindlessResourceTable.cpp` 생성 — `registerTexture/Buffer()`, `commit()` 구현
21. `include/struct/ubo.h` 수정 — `DrawResourceIndices` 구조체 추가
22. `shader/pbrForward.frag` 수정 — `textures[]` bindless 배열로 전환
23. `shader/pbrForward.vert` 수정 — push constant 구조체 맞춤
24. `include/component2/VKrenderer.h` 수정 — `BindlessResourceTable` 멤버 추가
25. `app/component2/VKrenderer.cpp` 수정 — 모델 로드 시 `registerTexture()`, 드로우 루프에서 인덱스 push constant 전달
26. **검증**: `graph.printGraph()` + 1회 descriptor set 바인딩 확인, 시각적 결과 동일한지 확인

---

## 목표 5: TextureManager — IShaderResource 기반 bindless 텍스처 관리

> **HonglabVulkan 예시 참고**: 예시의 `TextureManager : Resource` 패턴을 그대로 채용.
> 별도의 `ResourceManager` 추상 계층 없이 `TextureManager`가 `IShaderResource`를 직접 상속.
> `DescriptorSetHander::create()` 에 다른 UBO들과 함께 순서대로 전달하면 자동 바인딩됨.

### 핵심 아이디어 (예시에서 배운 점)

```
예시:  TextureManager : Resource       → updateBinding(MAX) + updateWrite(actual)
우리:  TextureManager : IShaderResource → 동일 패턴
```

`TextureManager` 자체가 **하나의 descriptor binding 슬롯** 역할을 하고,
내부적으로 모든 텍스처를 배열로 보유한다:
- `updateBinding()` → layout에 `descriptorCount = MAX_TEXTURES` (상한) 선언
- `updateWrite()`   → 실제 `textures.size()` 개만 imageInfo 배열로 채워서 GPU에 반영

### 새 파일: `include/engine2/TextureManager.h`

```cpp
namespace vkengine {

class TextureManager : public IShaderResource {
public:
    static constexpr uint32_t MAX_TEXTURES = 512;

    explicit TextureManager(VKcontext& ctx);

    // 이름과 함께 텍스처 추가 (non-owning) → 삽입 순서 = bindless 인덱스
    // e.g. add("albedo", &model.textures[0])  → returns 0
    //      add("normal", &model.textures[1])  → returns 1
    uint32_t add(const std::string& name, VKImage2D* tex);

    // 이름으로 텍스처 포인터 조회
    VKImage2D* get(const std::string& name) const;

    // 이름으로 GPU 배열 내 인덱스 조회 (push constant에 전달)
    uint32_t getIndex(const std::string& name) const;

    size_t size() const { return textures.size(); }

    // IShaderResource 구현 ─────────────────────────────────────────

    // layout 선언: MAX_TEXTURES 슬롯의 CombinedImageSampler 배열
    void updateBinding(VkDescriptorSetLayoutBinding& binding) override {
        binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = MAX_TEXTURES;   // layout 상한
        binding.stageFlags      = 0;              // ShaderManager가 주입
    }

    // 실제 데이터: 등록된 텍스처 수(textures.size())만큼만 write
    void updateWrite(VkWriteDescriptorSet& write) override;

    // RenderGraph 배리어용 (TextureManager는 단일 barrier 없으므로 nullptr 반환)
    VKResourceBinding& getResourceBinding() override       { return dummyBinding; }
    const VKResourceBinding& getResourceBinding() const override { return dummyBinding; }

private:
    // 삽입 순서 = bindless 인덱스
    std::vector<std::pair<std::string, VKImage2D*>>   textures;  // 순서 있는 목록
    std::unordered_map<std::string, uint32_t>          nameToIdx; // name → 인덱스

    mutable std::vector<VkDescriptorImageInfo> imageInfos; // updateWrite() 내부용
    VKResourceBinding dummyBinding;  // getResourceBinding() 반환용 (미사용)
};

} // namespace vkengine
```

### `updateWrite()` 구현

```cpp
void TextureManager::updateWrite(VkWriteDescriptorSet& write) {
    imageInfos.resize(textures.size());
    for (size_t i = 0; i < textures.size(); ++i) {
        // VKImage2D의 resourceBinding에서 imageInfo 가져오기
        imageInfos[i] = textures[i].second->getResourceBinding().imageInfo;
    }

    write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = uint32_t(textures.size()); // MAX_TEXTURES 아닌 실제 수
    write.pImageInfo      = imageInfos.data();
    // dstSet / dstBinding / dstArrayElement 은 DescriptorSetHander::create()가 설정
}
```

### DescriptorSetHander::create() 와의 통합

```cpp
// Renderer에서 descriptor set 생성 시 — 순서가 binding 번호가 됨
DescriptorSetHander sceneSet;
sceneSet.create(ctx, {
    std::ref(sceneUBO),      // binding=0: uScene
    std::ref(skyUBO),        // binding=1: uSkyOptions
    std::ref(optionsUBO),    // binding=2: uOptions
    std::ref(textureManager) // binding=3: textures[MAX_TEXTURES]
    //  ↑ layout에는 MAX_TEXTURES, write에는 실제 등록 수 자동 처리
});
```

### VKModel과의 통합

`VKModel::textures`를 `TextureManager`로 교체:

```cpp
class VKModel {
public:
    TextureManager textureManager; // 이름 + 인덱스 + GPU 데이터 일체화

    // VKModelLoader에서 텍스처 로드 후:
    // textureManager.add("albedo",            &rawTextures[mat.baseColorIdx]);
    // textureManager.add("normal",            &rawTextures[mat.normalIdx]);
    // textureManager.add("metallicRoughness", &rawTextures[mat.metallicRoughnessIdx]);
};
```

드로우 시:
```cpp
DrawResourceIndices drawData{};
drawData.modelMatrix  = mesh.transform;
drawData.albedoIdx    = model.textureManager.getIndex("albedo");    // → 0
drawData.normalIdx    = model.textureManager.getIndex("normal");    // → 1
vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_ALL, 0, sizeof(drawData), &drawData);
```

### Phase 5: TextureManager 구현 순서 (Phase 2, 4 이후)

27. `include/engine2/TextureManager.h` 생성 — `TextureManager : IShaderResource` 선언
28. `app/engine2/TextureManager.cpp` 생성 — `add()`, `get()`, `getIndex()`, `updateWrite()` 구현
29. `include/engine2/VKModel.h` 수정 — `TextureManager textureManager` 멤버 추가
30. `app/engine2/VKModelLoader.cpp` 수정 — 텍스처 로드 시 `textureManager.add()` 호출
31. `app/component2/VKrenderer.cpp` 수정 — `sceneSet.create(ctx, {..., textureManager})` 로 변경, 드로우 루프에서 `getIndex()` 사용
32. **검증**: 기존 텍스처 렌더링 결과 동일, `textureManager.size()` 출력으로 등록 수 확인

---

## 전체 구조 관계도

```
SPIRV-Reflect
    ↓
ShaderResourceLayout     ← 셰이더 파이프라인별 바인딩 이름+타입 정보
    ↓
IShaderResource          ← updateBinding() + updateWrite() + getResourceBinding()
    ├── VKBaseBuffer2    ← 단일 UBO/SSBO
    ├── VKImage2D        ← 단일 텍스처
    └── TextureManager   ← bindless 텍스처 배열 (MAX 선언 / 실제 수 write)
            ↓
DescriptorSetHander::create(vector<IShaderResource&>)
    ← 순서 = binding 번호, stageFlags는 ShaderManager가 SPIRV-Reflect로 자동 주입
            ↓
DrawResourceIndices (push constant)
    ← textureManager.getIndex("albedo") 등 이름→인덱스 변환
            +
RenderGraph              ← 패스 실행 순서 결정 + 배리어 자동 발행
```

---

## 수정 파일 전체 요약 (Phase 1~5)

### 새 파일 생성

| 파일 | 내용 |
|------|------|
| `include/engine2/IShaderResource.h` | `updateBinding()`, `updateWrite()`, `getResourceBinding()` 순수 가상 |
| `include/engine2/ShaderResourceLayout.h` | `BindingDesc`, `PushConstantDesc`, `ShaderResourceLayout` |
| `app/engine2/ShaderResourceLayout.cpp` | `findBinding()`, `findBindingByName()` |
| `include/engine2/RenderGraph.h` | `ResourceAccess`, `RenderPassNode`, `RenderGraph` |
| `app/engine2/RenderGraph.cpp` | 위상 정렬, 배리어 삽입, 실행 루프 |
| `include/engine2/TextureManager.h` | `TextureManager : IShaderResource` — bindless 배열 + 이름 추적 |
| `app/engine2/TextureManager.cpp` | `add()`, `getIndex()`, `updateWrite()` 구현 |

### 기존 파일 수정

| 파일 | 변경 내용 |
|------|-----------|
| `include/engine2/VKbuffer2.h` | `IShaderResource` 상속, `updateBinding()` / `updateWrite()` / `getResourceBinding()` override |
| `include/engine2/VKImage2D.h` | `IShaderResource` 상속, 위 3개 메서드 override |
| `include/engine2/VKDescriptorSet.h` | `create(vector<reference_wrapper<IShaderResource>>)` 오버로드 추가 |
| `include/engine2/VKShaderManager.h` | `resourceLayouts`, `buildResourceLayouts()`, `getResourceLayout()` 추가 |
| `app/engine2/VKShaderManager.cpp` | `buildResourceLayouts()` 구현, constructor에 호출 추가 |
| `include/engine2/VKpipeLineHandle.h` | `getResourceLayout()` 선언 |
| `app/engine2/VKpipeLineHandle.cpp` | `getResourceLayout()` 구현 |
| `app/engine2/VKdeviceHandler2.cpp` | descriptor indexing 기능 활성화 |
| `include/struct/ubo.h` | `DrawResourceIndices` 추가 |
| `shader/pbrForward.frag` | `textures[]` bindless 배열로 전환 |
| `shader/pbrForward.vert` | push constant 구조체 변경 |
| `include/component2/VKrenderer.h` | `TextureManager` 멤버, 이미지 getter 추가 |
| `app/component2/VKrenderer.cpp` | `create()` 패턴으로 descriptor set 생성, 드로우 루프 인덱스 전달 |
| `include/engine2/VKModel.h` | `TextureManager textureManager` 멤버 추가 |
| `app/engine2/VKModelLoader.cpp` | 텍스처 로드 시 `textureManager.add()` 호출 |

---

## 주의사항

- `VKBarrierHelper`는 non-owning pointer로 저장 — 원본 리소스 수명이 graph보다 길어야 함
- 스왑체인 이미지는 매 프레임 바뀌므로 `execute()`에 `VkImage swapchainImage`를 매번 전달하여 갱신
- 기존 renderer 내부의 inline `transitionTo()` 호출은 graph가 배리어 발행 후 실행되면, `VKBarrierHelper::transitionImageLayout2()`의 "동일 레이아웃이면 skip" 최적화로 자동 no-op → 첫 통합 시 안전하게 공존 가능
- Bindless 전환 시 셰이더 재컴파일 필수 — `glslc`로 `.spv` 재생성 후 SPIRV-Reflect로 검증
- `MAX_TEXTURES` / `MAX_BUFFERS` 초과 등록 시 assert 처리 필요
