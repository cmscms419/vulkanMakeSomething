#ifndef INCLUDE_RESOURSELOAD_H_
#define INCLUDE_RESOURSELOAD_H_

#include <stdlib.h>
#include <png.h>

enum
{
    Texture_default = 0, // only used for desired_channels
    Texture_grey = 1,
    Texture_grey_alpha = 2,
    Texture_rgb = 3,
    Texture_rgb_alpha = 4
} TextureType;

unsigned char* load_png_rgba(const char* filename, int* width, int* height, int type);

#endif // !INCLUDE_RESOURSELOAD_H_
