#include "image.hpp"
#include "irvm_math.hpp"
#include "origin.hpp"
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace irvm {
    IndexedImage index_image(const OriginImage &origin, const Palette &palette) {
        if(palette.size < 2)
            throw std::runtime_error("Invalid palette");

        IndexedImage indexed;
        indexed.width = origin.width;
        indexed.height = origin.height;

        indexed.pixels.resize(
            static_cast<size_t>(origin.width) *
            static_cast<size_t>(origin.height)
        );

        std::vector<Lab> palette_lab(palette.size);
        for(uint8_t i = 0; i < palette.size; ++i) {
            palette_lab[i] = rgb_to_lab(
                palette.colors[i].r,
                palette.colors[i].g,
                palette.colors[i].b
            );
        }

        //
        // Find the closest palette color for every pixel.
        //
        for(size_t i = 0; i < origin.pixels.size(); ++i) {
            const OriginPixel& origin_px = origin.pixels[i];
            const Lab origin_px_lab = rgb_to_lab(
                origin_px.r, 
                origin_px.g,
                origin_px.b
            );

            double best_distance = std::numeric_limits<double>::max();
            uint8_t best_idx = 0; // FIXME

            for(uint8_t p = 0; p < palette.size; ++p) {
                const double curr_distance = distance_squared(origin_px_lab, palette_lab[p]);
                if(curr_distance < best_distance) {
                    best_distance = curr_distance;
                    best_idx = p;
                }
            }

            indexed.pixels[i] = best_idx;
        }

        return indexed;
    }
};
