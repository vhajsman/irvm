#include "image.hpp"
#include "irvm_math.hpp"
#include "origin.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace irvm {
    IndexedImage index_image(const OriginImage &origin, const Palette &palette) {
        if(palette.size < 2)
            throw std::runtime_error("Invalid palette");

        const size_t pixel_count = origin.width * origin.height;

        if(origin.pixels.size() != pixel_count)
            throw std::runtime_error("Origin pixel count does not match image dimensions");

        IndexedImage indexed;
        indexed.width = origin.width;
        indexed.height = origin.height;

        indexed.pixels.resize(pixel_count);

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
//
//            double best_distance = std::numeric_limits<double>::max();
//            uint8_t best_idx = 0; // FIXME
//
//            for(uint8_t p = 0; p < palette.size; ++p) {
//                const double curr_distance = distance_squared(origin_px_lab, palette_lab[p]);
//                if(curr_distance < best_distance) {
//                    best_distance = curr_distance;
//                    best_idx = p;
//                }
//            }
//
//            indexed.pixels[i] = best_idx;
            
            double best_dist = std::numeric_limits<double>::infinity();
            size_t best_idx = 0;
            bool found = 0;

            for(size_t p = 0; p < static_cast<size_t>(palette.size); ++p) {
                const double dist = distance_squared(origin_px_lab, palette_lab[p]);
                if(!std::isfinite(dist))
                    throw std::runtime_error("NaN or Infinity in Lab color dist");

                if(dist < best_dist) {
                    best_dist = dist;
                    best_idx = p;
                    found = true;
                }
            }

            if(!found)
                throw std::runtime_error("Could not find closest palette color");

            indexed.pixels[i] = static_cast<uint8_t>(best_idx);
        }

        return indexed;
    }
};
