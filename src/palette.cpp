#include "image.hpp"
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace irvm {
    Palette build_palette(const std::vector<ColorCluster>& clusters, uint8_t palette_size) {
        if(palette_size == 0)
            throw std::runtime_error("Palette cannot be empty.");
        if(palette_size > MAX_PALETTE_SIZE)
            throw std::runtime_error("Too many colors in palette.");

        // NOTE: This function assumes clusters are already sorted
        // by importance / pixel count.

        Palette palette {};
        palette.size = static_cast<uint8_t>(std::min<size_t>(palette_size, clusters.size()));

        for(uint8_t i = 0; i < palette.size; ++i) {
            palette.colors[i].r = clusters[i].color.r;
            palette.colors[i].g = clusters[i].color.g;
            palette.colors[i].b = clusters[i].color.b;

            std::cout
                << "PALETTE " << static_cast<unsigned>(i) << "\t#" << std::hex
                << std::setw(2) << std::setfill('0') << static_cast<unsigned>(palette.colors[i].r)
                << std::setw(2) << static_cast<unsigned>(palette.colors[i].g)
                << std::setw(2) << static_cast<unsigned>(palette.colors[i].b)
                << std::dec << "\tCount: " << clusters[i].count << '\n';
        }

        return palette;
    }
}
