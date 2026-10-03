#include "origin.hpp"
#include "image.hpp"

#include <cstdio>
#include <png.h>
#include <stdexcept>

namespace irvm {
    namespace {
        struct PngFile {
            FILE* file;
            png_structp png_ptr;
            png_infop info_ptr;

            ~PngFile() {
                if(this->png_ptr != nullptr)
                    png_destroy_read_struct(&this->png_ptr, this->info_ptr != nullptr ? &this->info_ptr : nullptr, nullptr);

                if(this->file != nullptr)
                    fclose(this->file);
            }
        };
    };

    OriginImage load_origin_image(const char* filename) {
        PngFile png_file;
        png_file.file = fopen(filename, "rb");
        if(png_file.file == nullptr)
            throw std::runtime_error("Failed to open PNG file for reading.");

        png_file.png_ptr = png_create_read_struct(
            PNG_LIBPNG_VER_STRING,
            nullptr,
            nullptr,
            nullptr
        );

        if(png_file.png_ptr == nullptr) {
            throw std::runtime_error("Failed to create PNG read struct.");
        }

        png_file.info_ptr = png_create_info_struct(png_file.png_ptr);
        if(png_file.info_ptr == nullptr) 
            throw std::runtime_error("Failed to create PNG info struct.");

        if(setjmp(png_jmpbuf(png_file.png_ptr)))
            throw std::runtime_error("Error during PNG read.");

        png_init_io(png_file.png_ptr, png_file.file);
        png_read_info(png_file.png_ptr, png_file.info_ptr);

        const uint32_t width =  png_get_image_width(png_file.png_ptr, png_file.info_ptr);
        const uint32_t height = png_get_image_height(png_file.png_ptr, png_file.info_ptr);
        const int color_type =  png_get_color_type(png_file.png_ptr, png_file.info_ptr);
        const int bit_depth =   png_get_bit_depth(png_file.png_ptr, png_file.info_ptr);

        if(width == 0 || height == 0)
            throw std::runtime_error("PNG has invalid dimensions.");
        if(width > MAX_WIDTH || height > MAX_HEIGHT)
            throw std::runtime_error("PNG exceeds the maximum IRVM image dimensions.");
        
        // if(bit_depth != 8)
        //     throw std::runtime_error("Unsupported PNG bit depth. Only 8-bit images are supported.");
        // if(color_type != PNG_COLOR_TYPE_RGB)
        //     throw std::runtime_error("Unsupported PNG color type. Only 8-bit RGB images are supported.");

        if(bit_depth == 16)
            png_set_strip_16(png_file.png_ptr);
        if(color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png_file.png_ptr);
        if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand_gray_1_2_4_to_8(png_file.png_ptr);
        if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_file.png_ptr);

        png_read_update_info(png_file.png_ptr, png_file.info_ptr);

        OriginImage image;
        image.width = width;
        image.height = height;

        image.pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height));

        std::vector<png_bytep> rows(height);
        for(uint32_t y = 0; y < height; ++y) {
            rows[y] = reinterpret_cast<png_bytep>(image.pixels.data() + static_cast<size_t>(y) * width);
        }

        png_read_image(png_file.png_ptr, rows.data());
        png_read_end(png_file.png_ptr, nullptr);

        return image;
    }
};
