#include "resourseload.h"

namespace vkengine {

    cUChar* load_png_rgba(const cChar* filename, cUint32_t* width, cUint32_t* height, cInt type)
    {
        FILE* fp = NULL;
        fopen_s(&fp, filename, "rb");
        if (!fp) return NULL;

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
            fclose(fp);
            return NULL;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, NULL, NULL);
            fclose(fp);
            return NULL;
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            return NULL;
        }

        png_init_io(png_ptr, fp);
        png_read_info(png_ptr, info_ptr);

        *width = png_get_image_width(png_ptr, info_ptr);
        *height = png_get_image_height(png_ptr, info_ptr);

        // RGBA 형식으로 변환 설정
        png_set_expand(png_ptr);
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY ||
            png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);
        if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
            png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

        png_read_update_info(png_ptr, info_ptr);

        // 데이터 읽기
        png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * (*height));
        cUChar* image_data = (cUChar*)malloc((*width) * (*height) * 4);

        if (row_pointers == NULL || image_data == NULL) {
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

            fclose(fp);

            if (row_pointers) free(row_pointers);
            if (image_data) free(image_data);

            return NULL;
        }

        for (int y = 0; y < (*height); y++) {
            row_pointers[y] = image_data + y * (*width) * 4;
        }

        png_read_image(png_ptr, row_pointers);
        free(row_pointers);

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);

        return image_data;
    }
    
    void GetTextureSize(const cChar* filename, cUint32_t* width, cUint32_t* height)
    {
        FILE* fp = NULL;
        fopen_s(&fp, filename, "rb");
        if (!fp) return;
        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
            fclose(fp);
            return;
        }
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, NULL, NULL);
            fclose(fp);
            return;
        }
        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            return;
        }

        png_init_io(png_ptr, fp);
        png_read_info(png_ptr, info_ptr);
        *width = png_get_image_width(png_ptr, info_ptr);
        *height = png_get_image_height(png_ptr, info_ptr);

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        fclose(fp);
    }

} // namespace vkengine