#include "irvm_math.hpp"
#include <cmath>

namespace irvm {
    /**
     * @brief Convert an sRGB component to linear RGB.
     *
     * @param value sRGB
     * @return double 
     */
    double srgb_to_linear(uint8_t value) {
        //
        // PNG stores ordinary sRGB values. CIELAB conversion operates on
        // linear-light RGB, so undo the sRGB transfer function first.
        //
        // The constants below are defined by the sRGB / IEC 61966-2-1
        // transfer function.
        //

        const double c = static_cast<double>(value) / 255.0;
        return (c <= 0.04045) ? (c / 12.92) : std::pow((c + 0.055) / 1.055, 2.4);
    }

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
    Lab rgb_to_lab(uint8_t r, uint8_t g, uint8_t b) {
        const double rr = srgb_to_linear(r);
        const double gg = srgb_to_linear(g);
        const double bb = srgb_to_linear(b);

        //
        // These coefficients are the standard sRGB -> XYZ conversion
        // matrix for the D65 reference white.
        //
        // https://www.w3.org/Graphics/Color/srgb.pdf
        //

        const double x = rr * 0.4124564 + gg * 0.3575761 + bb * 0.1804375;
        const double y = rr * 0.2126729 + gg * 0.7151522 + bb * 0.0721750;
        const double z = rr * 0.0193339 + gg * 0.1191920 + bb * 0.9503041;

        //
        // XYZ values are normalized so that the reference white is:
        //
        // X = 0.95047
        // Y = 1.00000
        // Z = 1.08883
        //

        constexpr double xn = 0.95047;
        constexpr double yn = 1.00000;
        constexpr double zn = 1.08883;

        auto f = [](double value) {
            //
            // CIELAB uses a piecewise function when converting XYZ to Lab.
            //
            // The transition point is:
            //
            //     delta = 6 / 29
            //
            // This is part of the CIE 1976 L*a*b* definition.
            //

            constexpr double delta = 6.0 / 29.0;
            if(value > delta * delta * delta)
                return std::cbrt(value);

            return value / (3.0 * delta * delta) + 4.0 / 29.0;
        };

        const double fx = f(x / xn);
        const double fy = f(y / yn);
        const double fz = f(z / zn);

        Lab result;

        //
        // CIE 1976 L*a*b*:
        //
        //     L* = 116 f(Y/Yn) - 16
        //     a* = 500 [f(X/Xn) - f(Y/Yn)]
        //     b* = 200 [f(Y/Yn) - f(Z/Zn)]
        //

        result.l = 116.0 * fy - 16.0;
        result.a = 500.0 * (fx - fy);
        result.b = 200.0 * (fy - fz);

        return result;
    }

    double distance_squared(const Lab& a, const Lab& b) {
        const double dl = a.l - b.l;
        const double da = a.a - b.a;
        const double db = a.b - b.b;

        return dl * dl + da * da + db * db;
    }

    uint32_t color_key(uint8_t r, uint8_t g, uint8_t b) {
        return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
    }

    // Convert a Lab center back to RGB.
    ClusterColor lab_to_rgb(const Lab& lab) {
        //
        // XYZ values are normalized so that the reference white is:
        //
        // X = 0.95047
        // Y = 1.00000
        // Z = 1.08883
        //

        constexpr double xn = 0.95047;
        constexpr double yn = 1.00000;
        constexpr double zn = 1.08883;

        const double fy = (lab.l + 16.0) / 116.0;
        const double fx = lab.a / 500.0 + fy;
        const double fz = fy - lab.b / 200.0;

        auto finv = [](double value) {
            //
            // CIELAB uses a piecewise function when converting XYZ to Lab.
            //
            // The transition point is:
            //
            //     delta = 6 / 29
            //
            // This is part of the CIE 1976 L*a*b* definition.
            //

            constexpr double delta = 6.0 / 29.0;
            if(value > delta)
                return value * value * value;

            return 3.0 * delta * delta * (value - 4.0 / 29.0);
        };

        const double x = xn * finv(fx);
        const double y = yn * finv(fy);
        const double z = zn * finv(fz);

        // https://www.w3.org/Graphics/Color/srgb.pdf

        double r = x *  3.2404542 + y * -1.5371385 + z * -0.4985314;
        double g = x * -0.9692660 + y *  1.8760108 + z *  0.0415560;
        double b = x *  0.0556434 + y * -0.2040259 + z *  1.0572252;

        //
        // Inverse sRGB transfer function.
        //
        // These constants are the inverse of the IEC 61966-2-1
        // sRGB transfer function used in srgb_to_linear().
        //

        auto linear_to_srgb = [](double value) {
            if(value <= 0.0031308)
                return 12.92 * value;

            return 1.055 * std::pow(value, 1.0 / 2.4) - 0.055;
        };

        r = linear_to_srgb(r);
        g = linear_to_srgb(g);
        b = linear_to_srgb(b);

        auto clamp = [](double value) {
            return std::max(0.0, std::min(1.0, value));
        };

        ClusterColor result;
        result.r = static_cast<uint8_t>(std::lround(clamp(r) * 255.0));
        result.g = static_cast<uint8_t>(std::lround(clamp(g) * 255.0));
        result.b = static_cast<uint8_t>(std::lround(clamp(b) * 255.0));

        return result;
    }
};
