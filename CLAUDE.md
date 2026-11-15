# Vulkan Engine 개발 컨텍스트

## 전체 계획
docs/render-graph-plan.md 참고

## 현재 브랜치
basic02

## 완료된 작업
- Phase 1: ShaderResourceLayout (docs/render-graph-plan.md의 목표 1)
  - Bindinginfo, PushConstantinfo, VertexInputinfo, DescriptorSetLayout, ShaderResourceLayout 구조체
  - collectPerPipelineBindings() — descriptor/push constant/vertex input 수집
  - getShaderResourceLayout() 공개 API
  - 관련 파일: include/struct/descriptor.h, app/struct/descriptor.cpp,
               include/engine2/ShaderManager.h, app/engine2/ShaderManager.cpp,
               include/engine2/Shader.h, app/engine2/Shader.cpp

- Phase 2: ShaderResource 인터페이스 (목표 3 일부)
  - ShaderResource 순수 가상 클래스 (updateBinding/updateWrite/getResourceBinding)
  - BaseBuffer2, Image2D가 ShaderResource 상속
  - DescriptorSetHandle::create(vector<ShaderResource>) 오버로드 추가
  - 관련 파일: include/engine2/ShaderResource.h,
               include/engine2/BaseBuffer2.h, include/engine2/Image2D.h,
               include/engine2/DescriptorSetHandle.h

## 다음 작업 (Phase 3)
- Image2D의 updateBinding()/updateWrite() 구현 확인
- RenderGraph 구현 시작 (include/engine2/RenderGraph.h)
- VKShadowMap → Image2D 교체 (Phase 3과 함께)

## 주요 설계 결정
- Bindinginfo = name(cString) + VkDescriptorSetLayoutBinding (기존 VkDescriptorSetLayoutBinding 대체)
- collectLayoutInfos()는 Phase 1/2/3으로 분리됨
- VKShadowMap 삭제는 RenderGraph 구현 시 함께 진행
- LayoutInfo는 VkDescriptorSetLayoutBinding 유지 (이름은 Resourcelayouts에서 별도 관리)
