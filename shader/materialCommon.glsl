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
//   gPosition.a = 모델 번호 + (파라미터0 8비트, 파라미터1 8비트) / 65536
//   정수부 = 모델 번호, 소수부 16비트 = 파라미터 두 개 (각 0~1 을 0~255 로 양자화).
//   gPosition 은 RGBA32F 라 정수부가 작으면(< 128) 소수부 16비트가 정확히 보존된다.
//
// 모델별 G-buffer 해석
//   STANDARD : gAlbedo.a = metallic,     파라미터0 = 0,                파라미터1 = 0
//   CLOTH    : gAlbedo.a = sheen 세기,   파라미터0 = 얇은 천 투과 (0~1), 파라미터1 = metallic (0~1)
// G-buffer 를 쓰는 셰이더(pbrdeferred, physicsInstance, cloth)는 반드시 이 값을 써야 한다.
// ---------------------------------------------------------------------------
#define SHADING_MODEL_STANDARD 0
#define SHADING_MODEL_CLOTH 1

float packShadingModel(int model, float parameter0, float parameter1) {
    float quantized0 = floor(clamp(parameter0, 0.0, 1.0) * 255.0 + 0.5);
    float quantized1 = floor(clamp(parameter1, 0.0, 1.0) * 255.0 + 0.5);
    return float(model) + (quantized0 * 256.0 + quantized1) / 65536.0;
}

int unpackShadingModel(float packed) {
    return int(floor(packed));
}

float unpackShadingParameter0(float packed) {
    float bits = floor(fract(packed) * 65536.0 + 0.5);
    return floor(bits / 256.0) / 255.0;
}

float unpackShadingParameter1(float packed) {
    float bits = floor(fract(packed) * 65536.0 + 0.5);
    return mod(bits, 256.0) / 255.0;
}

#endif // MATERIAL_COMMON_GLSL
