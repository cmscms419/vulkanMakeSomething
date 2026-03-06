# RenderGraph 구현 노트

## 개요

`RenderGraph`는 렌더 패스의 실행 순서와 이미지 배리어를 자동으로 관리하는 클래스다.
기존 `VKforwardRenderer::draw()` / `makeShadowMap()` 직접 호출을 대체한다.

---

## 핵심 구조

### ResourceEntry

```cpp
struct ResourceEntry
{
    VKImage2D*   image     = nullptr;  // 일반 이미지 (non-owning)
    VKSwapChain* swapchain = nullptr;  // 스왑체인 (non-owning)
};
```

- raw pointer 이므로 소멸자에서 `delete` 호출 없음 — **소유권 없음**
- `VKImage2D` 안에 `VKBarrierHelper`가 포함되어 있으므로 별도 barrier 멤버 불필요
- 스왑체인은 `VKSwapChain`이 `imageIndex`별 `VKBarrierHelper`를 내부 관리
- **수명 주의**: `ResourceEntry`가 살아있는 동안 원본 객체가 먼저 소멸되면 dangling pointer

### RenderPassNode

```cpp
struct RenderPassNode
{
    cString name;
    std::vector<ResourceUsage> inputs;   // 이 패스가 읽는 리소스
    std::vector<ResourceUsage> outputs;  // 이 패스가 쓰는 리소스
    PassExecuteFunc execute;             // 실제 렌더링 콜백
};
```

### ResourceUsage / ResourceAccess

```cpp
struct ResourceUsage {
    cString        handle;  // "shadowDepth", "forwardColor", "swapchain"
    ResourceAccess access;
};

enum class ResourceAccess : cUint16_t {
    ColorAttachmentWrite,
    DepthAttachmentWrite,
    ShaderReadOnly,
    ShaderReadWrite,  // compute general
    Present,
};
```

---

## compile() 흐름

`addPass()` 후 한 번만 호출. `sortedOrder`를 채워넣는다.

```
1. 유효성 검사  — 모든 input/output handle이 resources에 등록되어 있는지 확인
2. buildAdjacency()  — 의존성 그래프(인접 리스트) 생성
3. topologicalSort() — Kahn's BFS로 실행 순서 결정 → sortedOrder
```

---

## buildAdjacency()

**역할**: "어떤 패스가 끝나야 어떤 패스가 시작될 수 있는지"를 인덱스로 표현한 인접 리스트 생성

**반환**: `vector<vector<cSize>>` — `adj[i] = {j, k, ...}` 의미: 패스 i가 끝나야 패스 j, k가 시작 가능

**알고리즘**:

```
1단계 — outputs 등록:
  outputResources[handle] = {패스 인덱스들}

2단계 — inputs 기준으로 간선 추가:
  패스 i의 input handle을 outputResources에서 찾아
  → adj[src].push_back(i)  // src가 먼저, i가 나중
```

**예시**:
```
shadow  (outputs: "shadowDepth")
forward (inputs: "shadowDepth", outputs: "forwardColor")
post    (inputs: "forwardColor")

outputResources["shadowDepth"]  = {0}
outputResources["forwardColor"] = {1}

adj[0] = {1}   // shadow → forward
adj[1] = {2}   // forward → post
adj[2] = {}
```

**왜 `vector`인가** (vs `unordered_map`):
- 패스 인덱스가 `0..N-1` 연속이므로 직접 인덱스 접근이 최적
- 연속 메모리로 캐시 친화적
- `unordered_map`은 키가 sparse(불연속)할 때 유리

---

## topologicalSort() — Kahn's BFS

**역할**: 인접 리스트를 받아 렌더링 실행 순서(`sortedOrder`)를 생성

**알고리즘**:

```
1. inDegree[i] 계산 — 패스 i로 들어오는 엣지 수
2. inDegree == 0인 패스를 queue에 삽입 (의존성 없음 = 즉시 실행 가능)
3. BFS:
   cur = queue.pop()
   sorted.push_back(cur)
   for next in adj[cur]:
       if --inDegree[next] == 0: queue.push(next)
4. 사이클 감지: sorted.size() != passes.size() → 오류
```

**예시 결과**:
```
adj[0]={1}, adj[1]={2}, adj[2]={}
→ sortedOrder = {0, 1, 2}  // shadow → forward → post
```

---

## execute() 흐름 (미구현)

```cpp
// sortedOrder 순서대로:
for (cSize i : sortedOrder) {
    insertBarriersBeforePass(cmd, passes[i]);  // 배리어 삽입
    passes[i].execute(cmd, frameIndex);         // 패스 실행
}
```

### 스왑체인 처리
매 프레임 `execute()` 진입 시 스왑체인 이미지 갱신 필요:
```cpp
// resources[swapchainHandle].swapchain->getSwapChainImage(imageIndex) 로 접근
```

### insertBarriersBeforePass()
```cpp
// entry.swapchain != nullptr → swapchain->transitionTo(cmd, imageIndex, ...)
// entry.image != nullptr     → image->transitionTo(cmd, ...)
// VKBarrierHelper::transitionImageLayout2()의 "동일 레이아웃 skip" 최적화 내장
//  → renderer 내부 inline barrier와 중복되어도 자동 no-op
```

---

## 사용 예시

```cpp
// 초기화 시 (한 번)
RenderGraph graph(ctx);

graph.registerResource("shadowDepth",  renderer.getShadowMap());
graph.registerResource("forwardColor", renderer.getForwardToCompute());
graph.registerSwapchainResource("swapchain", swapchain);

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
        renderer.draw(cmd, frame, swapchainView, models, viewport, scissor);
    }
});

graph.addPass({
    .name    = "post",
    .inputs  = {{"forwardColor", ResourceAccess::ShaderReadOnly}},
    .outputs = {{"swapchain",    ResourceAccess::Present}},
    .execute = [&](VkCommandBuffer cmd, uint32_t frame) {
        renderer.drawPost(cmd, frame, swapchainView, viewport, scissor);
    }
});

graph.compile();  // 한 번만

// 매 프레임
graph.execute(cmd, frameIndex, swapchainImage, swapchainView);
```

---

## 관련 파일

| 파일 | 역할 |
|------|------|
| `include/engine2/VKRenderGraph.h` | 클래스/구조체 선언 |
| `app/engine2/VKRenderGraph.cpp` | `compile()`, `buildAdjacency()`, `topologicalSort()`, `execute()` 구현 |
| `include/struct/config.h` | `ResourceAccess`, `ResourceUsage` 정의 |
| `include/engine2/VKbarrier2.h` | `VKBarrierHelper::transitionImageLayout2()` |
| `include/component2/VKrenderer.h` | `getShadowMap()`, `getForwardToCompute()` getter 필요 |
