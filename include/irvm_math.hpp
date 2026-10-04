#ifndef IRVM_MATH_HPP
#define IRVM_MATH_HPP

#include "origin.hpp"
#include <cstdint>

namespace irvm {
    struct Lab {
        double l;
        double a;
        double b;
    };

    struct ColorSample {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint32_t count;
        Lab lab;
    };

    struct Cluster {
        Lab centroid;
        uint64_t pixel_count;
    };

    /**
     * @brief Convert an sRGB component to linear RGB.
     *
     * @param value sRGB
     * @return double 
     */
    double srgb_to_linear(uint8_t value);

    /**
     * @brief Converts RGB to CIELAB
     *
     * D65 white point
     * 
     * @param r 
     * @param g 
     * @param b 
     * @return Lab 
     */
    Lab rgb_to_lab(uint8_t r, uint8_t g, uint8_t b);

    double distance_squared(const Lab& a, const Lab& b);
    uint32_t color_key(uint8_t r, uint8_t g, uint8_t b);
    ClusterColor lab_to_rgb(const Lab& lab);
};

#endif
