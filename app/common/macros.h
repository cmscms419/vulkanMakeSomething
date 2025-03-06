#ifndef INCLUDE_MACROS_H_
#define INCLUDE_MACROS_H_

#include "common.h"

#include <algorithm>
#include <vector>
#include <string>


constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr int MAX_FRAMES = 4;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr int CREATESURFACE_VKWIN32SURFACECREATEINFOKHR = 0;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
};

const std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

const std::string MODEL_PATH = "../../../../../../source/viking_room.obj";
const std::string TEXTURE_PATH = "../../../../../../source/viking_room.png";

#ifdef DEBUG_
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

constexpr int UNIQUE_VERTEXTYPE = 1;

#define CHECK_RESULT(f)                                                  \
{                                                                        \
    bool res = (f);                                                      \
    if (res != true) {                                                   \
      printf("Fatal : function is %d in %s at line %d\n", res, __FILE__, \
             __LINE__);                                                  \
      assert(res == true);                                               \
    }                                                                    \
  }                                                                      \

#define VK_CHECK_RESULT(f)                                               \
{                                                                        \
    VkResult res = (f);                                                  \
    if (res != VK_SUCCESS) {                                             \
      printf("Fatal : VkResult is %d in %s at line %d\n", res, __FILE__, \
             __LINE__);                                                  \
      assert(res == VK_SUCCESS);                                         \
    }                                                                    \
  }                                                                      \

#define PRINT_TO_CONSOLE(text)              \
{                                           \
    printf("%s \n", &text);                 \
}                                           \

#endif // !INCLUDE_MACROS_H_
