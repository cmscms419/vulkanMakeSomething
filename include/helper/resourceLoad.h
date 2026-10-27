#ifndef INCLUDE_RESOURSELOAD_H_
#define INCLUDE_RESOURSELOAD_H_

#define _CRT_SECURE_NO_WARNINGS_

#include "common.h"

#include <ktx.h>

enum TextureType
{
    TextureDefault = 0, // only used for desired_channels
    TextureGrey = 1,
    TextureGreyAlpha = 2,
    TextureRGB = 3,
    TextureRGBAlpha = 4
};

cUChar* loadPNGAsRGBA(const cChar* filename, cUint32_t* width, cUint32_t* height, cInt type);

ktxTexture* loadKTXTexture(const cChar* filename, ktxTexture* texture);
ktxResult loadKTX2Texture(const cChar* filename, ktxTexture2* texture);

void getTextureSize(const cChar* filename, cUint32_t* width, cUint32_t* height);

#endif // !INCLUDE_RESOURSELOAD_H_
