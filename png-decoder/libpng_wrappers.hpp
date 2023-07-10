#pragma once

#include <cstdio>
#include <png.h>

#include "image.hpp"

namespace libpng {

    class StorageWrapper {
    public:
        StorageWrapper(int height, int row_bytes) : height_(height) {

            bytes_ = static_cast<png_bytep *>(malloc(sizeof(png_bytep) * height_));
            for (int y = 0; y < height_; y++) {
                bytes_[y] = static_cast<png_byte *>(malloc(row_bytes));
            }
        }

        png_byte *GetPixel(int row, int col) {
            return &bytes_[row][col * 4];
        }

        png_bytep *GetStorage() {
            return bytes_;
        }

        ~StorageWrapper() {
            for (int i = 0; i < height_; ++i) {
                free(bytes_[i]);
            }
            free(bytes_);
        }

    private:
        png_bytep *bytes_;
        int height_;
    };

    inline Image ReadImage(std::string_view filename) {
        FILE *fp = fopen(filename.data(), "rb");
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            throw std::runtime_error("Can't create png struct");
        }
        png_infop info = png_create_info_struct(png);
        if (!info) {
            throw std::runtime_error("Can't create png info");
        }
        if (setjmp(png_jmpbuf(png))) {
            throw std::runtime_error("libpng error");
        }

        png_init_io(png, fp);
        png_read_info(png, info);

        int width = png_get_image_width(png, info);
        int height = png_get_image_height(png, info);
        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth = png_get_bit_depth(png, info);

        // Read any color_type into 8bit depth, RGBA format.
        // See http://www.libpng.org/pub/png/libpng-manual.txt

        if (bit_depth == 16) {
            png_set_strip_16(png);
        }

        if (color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(png);
        }

        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
            png_set_expand_gray_1_2_4_to_8(png);
        }

        if (png_get_valid(png, info, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png);
        }

        // These color_type don't have an alpha channel then fill it with 0xff.
        if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
        }

        if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
            png_set_gray_to_rgb(png);
        }

        png_read_update_info(png, info);

        StorageWrapper storage(height, png_get_rowbytes(png, info));
        png_read_image(png, storage.GetStorage());
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);

        Image result(height, width);
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                auto data = storage.GetPixel(i, j);
                result(i, j) = RGB{data[0], data[1], data[2], data[3]};
            }
        }
        return result;
    }

    inline void WriteImage(const Image &image, std::string_view filename) {
        FILE *fp = fopen(filename.data(), "wb");
        if (!fp) {
            throw std::runtime_error("Can't open file for writing: " + std::string(filename));
        }

        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            throw std::runtime_error("Can't create png struct");
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            throw std::runtime_error("Can't create png info");
        }

        if (setjmp(png_jmpbuf(png))) {
            throw std::runtime_error("libpng error");
        }

        png_init_io(png, fp);

        // Output is 8bit depth, RGBA format.
        png_set_IHDR(png, info, image.Width(), image.Height(), 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png, info);

        // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
        // Use png_set_filler().
        // png_set_filler(png, 0, PNG_FILLER_AFTER);

        StorageWrapper storage(image.Height(), image.Width() * 4);
        for (int i = 0; i < image.Height(); ++i) {
            for (int j = 0; j < image.Width(); ++j) {
                auto data = storage.GetPixel(i, j);
                const auto &p = image(i, j);
                data[0] = p.r;
                data[1] = p.g;
                data[2] = p.b;
                data[3] = p.a;
            }
        }

        png_write_image(png, storage.GetStorage());
        png_write_end(png, nullptr);

        fclose(fp);
        png_destroy_write_struct(&png, &info);
    }

}// namespace libpng
