#ifndef __IRVM_ORIGIN_HPP__
#define __IRVM_ORIGIN_HPP__

#include <cstdint>
#include <vector>

namespace irvm {
    struct OriginPixel {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        // uint8_t a;
    };

    struct OriginImage {
        uint32_t width;
        uint32_t height;

        std::vector<OriginPixel> pixels;

        inline OriginPixel& pixel_at(uint32_t x, uint32_t y) { return pixels[y * width + x]; }
        inline const OriginPixel& pixel_at(uint32_t x, uint32_t y) const { return pixels[y * width + x]; }
    };

    OriginImage load_origin_image(const char* filename);

    // cluster.cpp

    struct ClusterColor {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct ColorCluster {
        ClusterColor color;
        uint32_t count;
    };

    std::vector<ColorCluster> cluster_colors(const OriginImage& image, uint16_t cluster_count);
};

#endif // __IRVM_ORIGIN_HPP__
