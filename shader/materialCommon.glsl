#ifndef MATERIAL_COMMON_GLSL
#define MATERIAL_COMMON_GLSL

// ---------------------------------------------------------------------------
// include/common/macros.h 의 같은 이름 define 과 값이 반드시 일치해야 한다.
// macros.h 는 <string> 과 namespace 를 쓰는 C++ 전용 헤더라 여기서 직접
// #include 할 수 없어서 한 벌 더 둔 것이다. 한쪽만 고치면 안 된다.
// ---------------------------------------------------------------------------

// 바인들리스 배열의 크기. C++ 쪽 TextureTable 이 같은 수만큼 슬롯을 잡는다.
#define MAX_TEXTURE_COUNT 512

// 0번 슬롯은 흰색 1x1 더미 텍스처다. 실제 텍스처는 1번부터 들어간다.
// 그래서 "텍스처가 없다" 는 판정은 index >= 0 이 아니라
// index > DUMMY_TEXTURE_INDEX 로 해야 한다.
#define DUMMY_TEXTURE_INDEX 0

// include/struct/materialData.h 의 enum class MaterialWorkflow 와 값이 같아야 한다.
#define MATERIAL_WORKFLOW_METALLIC_ROUGHNESS 0
#define MATERIAL_WORKFLOW_SPECULAR_GLOSSINESS 1

// ---------------------------------------------------------------------------
// 셰이딩 모델 — G-buffer 를 쓰는 셰이더가 gPosition.a 에 기록하고 lightdeferred 가 읽는다.
//   gPosition.a = 모델 번호 + 파라미터 * 0.5   (gPosition 은 RGBA32F 라 정밀도 문제가 없다)
//
// 모델별 G-buffer 해석
//   STANDARD : gAlbedo.a = metallic,     파라미터 = 0
//   CLOTH    : gAlbedo.a = sheen 세기,   파라미터 = 얇은 천 투과 (0~1)
// G-buffer 를 쓰는 셰이더(pbrdeferred, physicsInstance, cloth)는 반드시 이 값을 써야 한다.
// ---------------------------------------------------------------------------
#define SHADING_MODEL_STANDARD 0
#define SHADING_MODEL_CLOTH 1

float packShadingModel(int model, float parameter) {
    return float(model) + clamp(parameter, 0.0, 1.0) * 0.5;
}

int unpackShadingModel(float packed) {
    return int(floor(packed));
}

float unpackShadingParameter(float packed) {
    return fract(packed) * 2.0;
}

#endif // MATERIAL_COMMON_GLSL
