#ifndef INCLUDE_RESOURSELOAD_H_
#define INCLUDE_RESOURSELOAD_H_

#define _CRT_SECURE_NO_WARNINGS_

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common.h"

namespace vkengine {

    enum TextureType
    {
        Texture_default = 0, // only used for desired_channels
        Texture_grey = 1,
        Texture_grey_alpha = 2,
        Texture_rgb = 3,
        Texture_rgb_alpha = 4
    };

    cUChar* load_png_rgba(const cChar* filename, cUint32_t* width, cUint32_t* height, cInt type);

    void GetTextureSize(const cChar* filename, cUint32_t* width, cUint32_t* height);
}

#endif // !INCLUDE_RESOURSELOAD_H_
