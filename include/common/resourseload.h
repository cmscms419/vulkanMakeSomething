#ifndef INCLUDE_RESOURSELOAD_H_
#define INCLUDE_RESOURSELOAD_H_

#define _CRT_SECURE_NO_WARNINGS_

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

namespace vkengine {
    enum TextureType
    {
        Texture_default = 0, // only used for desired_channels
        Texture_grey = 1,
        Texture_grey_alpha = 2,
        Texture_rgb = 3,
        Texture_rgb_alpha = 4
    };

    unsigned char* load_png_rgba(const char* filename, uint32_t* width, uint32_t* height, int type);

    void GetTextureSize(const char* filename, uint32_t* width, uint32_t* height);
}

#endif // !INCLUDE_RESOURSELOAD_H_
