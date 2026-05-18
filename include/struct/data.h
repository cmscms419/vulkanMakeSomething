#ifndef INCLUDE_DATA_H
#define INCLUDE_DATA_H

#include "common.h"
#include "type.h"

constexpr cInt WIDTH = 1920;
constexpr cInt HEIGHT = 1080;
constexpr cInt MAX_FRAMES = 4;
constexpr cInt MAX_FRAMES_IN_FLIGHT = 2;
constexpr cInt MAX_FRAMES_IN_FLIGHT_UI_VERSION = 3;
constexpr cInt CREATESURFACE_VKWIN32SURFACECREATEINFOKHR = 0;
constexpr cInt MAX_BONES = 256; // 이 bone 수는 PC 기준으로 임시로 지정, 모바일 이나 다른 확경에서는 또 다르게 설정해야 한다

extern const std::vector<const char *> validationLayers;
extern const std::vector<const char *> coreDeviceExtensions;
extern const std::vector<VkDynamicState> dynamicStates;
extern const cString RESOURSE_PATH;
extern const cString SHADER_PATH;
extern const cString MODEL_PATH;
extern const cString TEXTURE_PATH;
extern const cString TEST_TEXTURE_PATH;
extern const cString TEST_TEXTURE_PATH_ARRAY0;
extern const cString TEST_TEXTURE_PATH_ARRAY1;
extern const cString TEST_TEXTURE_PATH_ARRAY2;
extern const cString TEST_TEXTURE_PATH_ARRAY3;
extern const cString TEST_TEXTURE_PATH_ARRAY4;
extern const cString TEST_TEXTURE_PATH_ARRAY5;
extern const cString CUBE_TEXTURE_PATH;
extern const cBool enableValidationLayers;

extern const std::vector<Vertex> cube;
extern const std::vector<Vertex> skyboxVertices;
extern const std::vector<Vertex> DepthTestVertices;
extern const std::vector<Vertex> SquareTestVertices;
extern const std::vector<cUint32_t> DepthTestIndices;
extern const std::vector<cUint32_t> SquareTestIndices_;
extern const std::vector<cUint32_t> cubeindices_;
extern const std::vector<cUint32_t> skyboxIndices;

#endif // INCLUDE_DATA_H