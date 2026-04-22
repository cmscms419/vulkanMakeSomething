# HonglabVulkan 참고 엔진 구조 개선 계획

> HonglabVulkan 프로젝트를 분석하여 현재 엔진과 비교한 개선 방향을 정리한 문서.
> render-graph-plan.md의 Phase 3 진행과 병행하여 적용 예정.

---

## 현재 계획에서 잘 되고 있는 것

현재 render-graph-plan.md는 HonglabVulkan의 핵심 패턴을 이미 제대로 따르고 있다.

| 현재 프로젝트 | HonglabVulkan 대응 |
|---|---|
| `IShaderResource` 추상 기반 | `Resource` 추상 기반 |
| `BarrierHelper` 재사용 | `BarrierHelper` 동일 패턴 |
| SPIRV-Reflect 기반 레이아웃 자동화 | `ShaderManager` 동일 방향 |

---

## 개선 항목

### 1. BarrierHelper 상태 추적 강화 (높음)

HonglabVulkan의 `BarrierHelper`는 현재 레이아웃/접근 플래그/파이프라인 스테이지를 **내부 상태로 추적**하여 중복 배리어를 자동으로 방지한다.

RenderGraph의 `insertBarriersBeforePass()`가 올바르게 동작하려면 `BarrierHelper`가 이 상태를 반드시 보유해야 한다.

```cpp
// BarrierHelper가 내부적으로 관리해야 할 멤버
VkImageLayout         currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
VkAccessFlags2        currentAccess = 0;
VkPipelineStageFlags2 currentStage  = VK_PIPELINE_STAGE_2_NONE;

// 배리어 삽입 전 중복 검사
bool needsTransition(VkImageLayout newLayout) const {
    return currentLayout != newLayout;
}
```

**확인 사항**: 현재 `VKBarrierHelper`가 이 상태를 실제로 저장하는지 점검.

---

### 2. Dynamic Rendering 전환 (높음)

HonglabVulkan은 `VkRenderPassBeginInfo` 없이 `vkCmdBeginRendering()`(VK_KHR_dynamic_rendering)을 사용한다. 전통적인 renderpass 객체를 제거하면 RenderGraph의 패스 관리가 훨씬 단순해진다.

**전환 시점**: VKShadowMap → VKImage2D 교체 + RenderGraph 구현과 **같은 PR**에서 진행.

```cpp
// 기존: VkRenderPassBeginInfo + vkCmdBeginRenderPass
// 변경 후:
VkRenderingAttachmentInfo colorAtt{
    .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView   = colorView,
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue  = { .color = {{0,0,0,1}} }
};

VkRenderingInfo renderInfo{
    .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea           = scissor,
    .layerCount           = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments    = &colorAtt,
    .pDepthAttachment     = &depthAtt
};

vkCmdBeginRendering(cmd, &renderInfo);
// ... draw calls ...
vkCmdEndRendering(cmd);
```

RenderPassNode의 `outputs`에서 attachment를 동적으로 구성하면 RenderGraph가 직접 attachment를 조립할 수 있다.

---

### 3. Per-Frame 리소스 이름 기반 인덱싱 (중간)

HonglabVulkan의 Renderer는 uniform buffer와 descriptor set을 **문자열 맵**으로 관리한다. RenderGraph의 각 패스가 리소스를 이름으로 참조하는 구조와 자연스럽게 맞물린다.

```cpp
// HonglabVulkan 패턴 — 현재 프로젝트에 적용
constexpr uint32_t kMaxFramesInFlight = 2;

unordered_map<string, array<VKBaseBuffer2, kMaxFramesInFlight>> perFrameUniformBuffers;
unordered_map<string, array<VKDescriptorSet, kMaxFramesInFlight>> perFrameDescriptorSets;

// 사용 시
perFrameUniformBuffers["sceneData"][frameIndex].update(sceneUBO);
perFrameDescriptorSets["shadow"][frameIndex].bind(cmd, pipelineLayout);
```

**장점**: RenderGraph 패스 등록 시 리소스 이름이 곧 식별자가 되어 `registerResource("sceneData", ...)` 패턴과 일관성이 유지된다.

---

### 4. compile() 단계 배리어 사전 계산 (낮음)

현재 계획은 `execute()` 시 매 프레임 배리어를 계산한다. `compile()` 단계에서 배리어를 **사전에 bake**해두면 실행 시 오버헤드가 줄어든다.

```cpp
// compile() 결과물
struct CompiledPass {
    RenderPassNode*               node;
    vector<VkImageMemoryBarrier2> barriersBeforePass; // 사전 계산
};
vector<CompiledPass> compiledPasses;

// execute() — 단순 제출
for (auto& cp : compiledPasses) {
    VkDependencyInfo depInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = (uint32_t)cp.barriersBeforePass.size(),
        .pImageMemoryBarriers    = cp.barriersBeforePass.data()
    };
    vkCmdPipelineBarrier2(cmd, &depInfo);
    cp.node->execute(cmd, frameIndex);
}
```

**주의**: 스왑체인 이미지처럼 프레임마다 핸들이 바뀌는 리소스는 execute() 시점에 동적 처리 필요.

---

### 5. Bindless Texture 지원 자리 확보 (낮음)

HonglabVulkan은 `TextureManager`로 최대 512개 텍스처를 단일 descriptor binding으로 관리한다. `VKShadowMap → VKImage2D` 교체 이후 텍스처가 늘어날 때를 대비해, 지금 설계에 자리를 남겨두는 것이 좋다.

```cpp
// 배열 descriptor — 단일 binding으로 N개 텍스처
VkDescriptorSetLayoutBinding textureArrayBinding{
    .binding         = TEXTURE_ARRAY_BINDING,
    .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .descriptorCount = MAX_TEXTURES,  // 512
    .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT
};

// 셰이더에서 인덱스로 접근
// layout(set=1, binding=0) uniform sampler2D textures[512];
// vec4 color = texture(textures[material.albedoIndex], uv);
```

---

### 6. View Frustum Culling (낮음)

HonglabVulkan은 `ViewFrustum.h`로 매 프레임 mesh 단위 AABB 컬링을 수행한다. RenderGraph 완성 후 추가할 Feature.

```cpp
// 매 프레임 카메라 VP 행렬로 frustum 갱신
ViewFrustum frustum;
frustum.update(camera.getViewProjection());

// forward pass 실행 직전 culling
for (auto& mesh : scene.meshes) {
    if (frustum.intersects(mesh.worldAABB)) {
        drawList.push_back(&mesh);
    }
}
```

---

## 적용 우선순위 및 시점

| 우선순위 | 항목 | 적용 시점 |
|---------|------|----------|
| **높음** | Dynamic Rendering 전환 | Phase 3 (RenderGraph + VKShadowMap 교체) 와 동일 PR |
| **높음** | BarrierHelper 상태 추적 완성 확인 | Phase 3 시작 전 점검 |
| **중간** | Per-frame 리소스 이름 맵 | Phase 3 RenderGraph 구현 중 |
| **낮음** | compile() 배리어 사전 계산 | RenderGraph 기능 완성 후 |
| **낮음** | Bindless Texture 자리 확보 | 텍스처 수 증가 시점 |
| **낮음** | View Frustum Culling | RenderGraph 완성 후 |

---

## Phase 3 권장 작업 묶음

아래 세 가지는 서로 의존성이 있어 같은 PR에서 처리하는 것이 중간 브레이킹 상태 없이 가장 깔끔하다.

```
[ Phase 3 단일 PR ]
├── VKShadowMap → VKImage2D 교체
├── Dynamic Rendering 전환 (VkRenderPass 제거)
└── RenderGraph 구현 (include/engine2/RenderGraph.h)
```
