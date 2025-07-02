# Vulkan_test

### vulkan을 사용해보는 일이 있어서 만져봅니다.
### 이전에 directx11에서 만든 3d cloth를 구현하는 것이 현재 목표
### 잘 만들면 이것을 통해서 이것저것을 만들 생각입니다.
### + 추가로 RenderDoc을 이용해서, shader Debug 가능

## 프로젝트를 실행하는데 필요한 것
- [visual studio 2022 버전 필요](https://visualstudio.microsoft.com/ko/vs/)
- [cuda toolkit 설치 12.6 버전](https://developer.nvidia.com/cuda-12-6-0-download-archive?target_os=Windows&target_arch=x86_64&target_version=11&target_type=exe_network)
- [vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- windows 11(windows 11에서만 실행 됨됨)

## 정리한 개인자료: https://www.notion.so/Vulkan-Tutorial-18118a41dc6f80c7adc7d7ba3f95542e?pvs=4
## vulkan_test 프로젝트에 computer shader에 대한 예제가 빠진 이유
https://vulkan-tutorial.com/Introduction 사이트를 참고해서 vulkan_test 프로젝트를
computer shader를 코드를 참고하기에는 프로젝트의 원형이 많이 손상될 것 같아서
Multisampling 챕터까지 완성했습니다. 
## 만든 것
[엔진 기본형](https://github.com/cmscms419/Vulkan_create_Somthing/tree/master/app/source/engine)

[사각형](https://github.com/cmscms419/Vulkan_create_Somthing/tree/master/app/source/Square)

[카메라 -> 엔진 기본에 적용 예정](https://github.com/cmscms419/Vulkan_create_Somthing/tree/master/app/source/camera)

[텍스처 입히기](https://github.com/cmscms419/Vulkan_create_Somthing/tree/master/app/source/texture)

[텍스처 백열 입히기 -> 육면체 각각면에 서로 다른 텍스처 입히기](https://github.com/cmscms419/Vulkan_create_Somthing/tree/master/app/source/textureArray)

[skymap](https://github.com/cmscms419/Vulkan_create_Somthing/tree/master/app/source/skymap) -> [skybox.vert, skybox.frag 출처](https://github.com/SaschaWillems/Vulkan/tree/master/shaders/glsl/texturecubemap)

[Descriptor 관련된 코드 정리 및 수정](https://github.com/cmscms419/vulkanMakeSomething/tree/master/app/source/DescriptorCodeUpdate)

[PBR Basice](https://github.com/cmscms419/vulkanMakeSomething/tree/master/app/source/PBRbasic) -> [PBR basice 출처, 쉐이더 출처](https://github.com/SaschaWillems/Vulkan/tree/master/examples/pbrbasic)

[image base lighting PBR](https://github.com/cmscms419/vulkanMakeSomething/tree/master/app/source/IBLPBR) -> [image base lighting PBR 코드 출처](https://github.com/SaschaWillems/Vulkan/tree/master/examples/pbribl)

## 텍스처 소스 출처 및 링크

[텍스처 소스 출처](https://opengameart.org/content/tiny-texture-pack-2)
[텍스처 홈페이지](https://opengameart.org/)
[skymap 텍스처 출처](https://learnopengl.com/Advanced-OpenGL/Cubemaps)
[GLTF-Sample](https://github.com/KhronosGroup/glTF-Sample-Models/tree/main)