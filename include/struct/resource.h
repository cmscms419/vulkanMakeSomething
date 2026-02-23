#ifndef INCLUDE_RESOURCE_H_
#define INCLUDE_RESOURCE_H_

#include "common.h"
#include "log.h"
#include "resourseload.h"

struct TextureResourceBase
{
    enum TYPE
    {
        NONE = 0,          // 리소스 없음
        PNG = 1,          // PNG 리소스
        KTX = 2,          // KTX 리소스
    };

    cChar* name = "";                              // 리소스 이름
    TYPE type = NONE;                               // 리소스 타입 (PNG, KTX 등)
    cUint32_t texWidth = 0;                         // 텍스처 너비
    cUint32_t texHeight = 0;                        // 텍스처 높이
    cUint32_t texChannels = 0;                      // 텍스처 채널
    cUint32_t mipLevels = 0;
    cUint32_t layerCount = 0;
    cUChar* data = nullptr;         //< 리소스 데이터 포인터

    virtual cBool createResource(
        cString path = "", 
        cChar* name = "", 
        TextureType textureType = TextureType::Texture_rgb_alpha) = 0; // 리소스 생성 함수, 경로를 인자로 받음
    virtual cBool createResource2(
        cString path = "", 
        cChar* name = "", 
        TextureType textureType = TextureType::Texture_rgb_alpha) = 0;
};

struct TextureResourcePNG : public TextureResourceBase {


    // 생성자
    TextureResourcePNG() {
        this->name = ""; // 이름 초기화
        this->texWidth = 0;
        this->texHeight = 0;
        this->texChannels = 0;
        this->data = nullptr; // 리소스 데이터 초기화
        this->type = TYPE::PNG; // 리소스 타입을 PNG로 설정
    }

    // 소멸자
    ~TextureResourcePNG() {

        if (data) {
            free(data); // 리소스 데이터 해제
        }
        texWidth = 0; // 너비 초기화
        texHeight = 0; // 높이 초기화
        texChannels = 0; // 채널 초기화
        data = nullptr; // 포인터 초기화
    }

    // 복사 생성자
    TextureResourcePNG(const TextureResourcePNG& other) {
        texWidth = other.texWidth;
        texHeight = other.texHeight;
        texChannels = other.texChannels;

        if (other.data) {
            size_t size = texWidth * texHeight * texChannels;
            data = (cUChar*)malloc(size);
            

            if (data && other.data) {
                memcpy(data, other.data, size);
            }
            else {
                vkengine::Log::EXIT_TO_LOGGER("Failed to allocate memory for texture data.");
            }
        }
        else {
            data = nullptr;
        }
    }

    // 대입 연산자
    TextureResourcePNG& operator=(const TextureResourcePNG& other) {
        if (this == &other) return *this;
        if (data) free(data);

        texWidth = other.texWidth;
        texHeight = other.texHeight;
        texChannels = other.texChannels;
        name = other.name;

        if (other.data) {
            size_t size = texWidth * texHeight * texChannels;
            data = (cUChar*)malloc(size);
            
            if (data && other.data) {
                memcpy(data, other.data, size);
            }
            else {
                vkengine::Log::EXIT_TO_LOGGER("Failed to allocate memory for texture data.");
            }

        }
        else {
            data = nullptr;
        }

        return *this;
    }

    virtual cBool createResource(cString path = "", cChar* name = "", TextureType textureType = TextureType::Texture_rgb_alpha) {

        this->name = name;

        if (data) {
            free(data);
        }

        this->texChannels = textureType; // 기본적으로 RGBA로 설정

        data = load_png_rgba(path.c_str(), &this->texWidth, &this->texHeight, this->texChannels);

        if (data == nullptr) {
            _PRINT_TO_CONSOLE_("PNG texture data is null.");
            return false;
        }

        return true;
    }
     cBool createResource2(cString path = "", cChar* name = "", TextureType textureType = TextureType::Texture_rgb_alpha) {
        return false;
    }
};

struct TextureResourceKTX : public TextureResourceBase {

    ktxTexture* texture = nullptr; // KTX 텍스처 포인터
    ktxTexture2* texture2 = nullptr; // KTX 텍스처 포인터

    TextureResourceKTX() {
        this->name = ""; // 이름 초기화
        this->texWidth = 0;
        this->texHeight = 0;
        this->texChannels = 0;
        this->mipLevels = 0;
        this->layerCount = 0;
        this->texture = nullptr; // KTX 텍스처 초기화
        this->type = TYPE::KTX;
    }

    ~TextureResourceKTX() {
        if (texture) {
            ktxTexture_Destroy(texture); // KTX 텍스처 해제
        }

        if (texture2)
        {
            ktxTexture_Destroy(ktxTexture(texture2));
        }

        texWidth = 0; // 너비 초기화
        texHeight = 0; // 높이 초기화
        texChannels = 0; // 채널 초기화
        mipLevels = 0;
        layerCount = 0;
        texture = nullptr; // 포인터 초기화
    }

    virtual cBool createResource(cString path = "", cChar* name = "", TextureType textureType = TextureType::Texture_rgb_alpha) {
        if (texture) {
            ktxTexture_Destroy(texture); // KTX 텍스처 해제
        }

        this->texture = load_ktx_texture(path.c_str(), texture);

        if (texture == nullptr) {
            _PRINT_TO_CONSOLE_("KTX texture is null.");
            return false;
        }

        this->texWidth = texture->baseWidth;
        this->texHeight = texture->baseHeight;
        this->mipLevels = texture->numLevels;
        this->layerCount = texture->numLayers;
        this->texChannels = 4; // 기본적으로 RGBA로 설정
        this->name = name;

        return true;
    }

    virtual cBool createResource2(cString path, cChar* name = "", TextureType textureType = TextureType::Texture_rgb_alpha) {
        
        if (this->texture2) {
            ktxTexture2_Destroy(this->texture2); // KTX 텍스처 해제
        }

        ktxResult result = ktxTexture2_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &this->texture2);

        if (result != KTX_SUCCESS)
        {
            vkengine::Log::EXIT_TO_LOGGER("Failed to load KTX2 texture: %s", path.c_str());
            return false;
        }

        this->texWidth = texture2->baseWidth;
        this->texHeight = texture2->baseHeight;
        this->mipLevels = texture2->numLevels;
        this->layerCount = texture2->numLayers;

        return true;
    }

};

#endif // !INCLUDE_RESOURCE_H_