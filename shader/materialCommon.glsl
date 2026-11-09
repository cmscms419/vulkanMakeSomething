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

#endif // MATERIAL_COMMON_GLSL
