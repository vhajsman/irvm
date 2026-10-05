#ifndef IRVM_IMAGE_HPP
#define IRVM_IMAGE_HPP

#include "origin.hpp"
#include <cstdint>
#include <vector>

namespace irvm {
    struct FileHeader;

    constexpr uint16_t MAX_WIDTH = UINT16_MAX;  ///< Maximum image width
    constexpr uint16_t MAX_HEIGHT = UINT16_MAX; ///< Maximum image height
    constexpr uint16_t MAX_PALETTE_SIZE = 24;   ///< Maximum colors per palette

    //
    // Coordinates
    // ---------------------
    //
    // Image coordinates:
    //     Used for positions on the shared image canvas.
    //
    // Vector coordinates:
    //     Used by vector geometry.
    //
    //     These are signed because glyphs can extend
    //     above/below the baseline and may use a larger logical coordinate
    //     system.
    //

    struct Coordinate {
        uint16_t x;
        uint16_t y;
    };

    /**
     * @brief Vector coordinates are expressed in font/object units. These are not pixel coordinates.
     * 
     */
    struct VectorCoordinate {
        int16_t x;
        int16_t y;
    };

    //
    // Palette and colors
    // ---------------------
    //
    
    struct PaletteColor {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct Palette {
        uint8_t size;
        PaletteColor colors[MAX_PALETTE_SIZE];
    };

    /**
     * @brief Generate color palette from color clusters
     * 
     * @param clusters color clusters
     * @param palette_size maximal size of palette that wont be exceeded
     * @return Palette 
     */
    Palette build_palette(const std::vector<ColorCluster>& clusters, uint8_t palette_size = MAX_PALETTE_SIZE); // palette.cpp

    //
    // Renderer instructions
    // ---------------------
    //

    enum class RendererCommand : uint8_t {
        place_sub_image,
        move_to,
        line_to,
        quadratic_bezier,
        close,
        fill_rect,
        stroke_rect,
        fill_triangle,
        stroke_triangle,
        set_color,
        set_line_width,
        translate,
        scale
    };

    struct PlaceSubImageParams   { Coordinate position; uint16_t object_index; };
    struct MoveToParams          { VectorCoordinate position; };
    struct LineToParams          { VectorCoordinate position; };
    struct QuadraticBezierParams { VectorCoordinate control, position; };
    struct RectParams            { VectorCoordinate p1, p2; };
    struct TriangleParams        { VectorCoordinate p1, p2, p3; };
    struct SetColorParams        { uint8_t color_index; };
    struct SetLineWidthParams    { uint8_t width; };
    struct TranslateParams       { int16_t x, y; };
    struct ScaleParams           { uint16_t x, y; };

    union RendererCommandParams {
        PlaceSubImageParams place_sub_image;
        MoveToParams move_to;
        LineToParams line_to;
        QuadraticBezierParams quadratic_bezier;
        RectParams rect;
        TriangleParams triangle;
        SetColorParams set_color;
        SetLineWidthParams set_line_width;
        TranslateParams translate;
        ScaleParams scale;
    };

    struct RendererInstruction {
        RendererCommand command;
        RendererCommandParams params;
    };

    //
    // Vector object
    // ---------------------
    //
    // An object is a sequence of renderer instructions.
    //
    // Width and height are NOT stored here. All objects in an IRVM file
    // share the dimensions specified by FileHeader.
    //

    enum class FillRule : uint8_t {
        non_zero,
        even_odd
    };

    struct VectorObject {
        const RendererInstruction* instructions;
        uint16_t instruction_count;

        FillRule fill_rule;
    };

    //
    // Icons
    // ---------------------
    //

    struct Icon {
        uint16_t id;
        VectorObject vector;
    };


    //
    // Glyphs
    // ---------------------
    //

    struct GlyphMetrics {
        int16_t advance_x;
        int16_t bearing_x;
        int16_t bearing_y;
    };

    struct Glyph {
        uint32_t codepoint;
        GlyphMetrics metrics;
        VectorObject vector;
    };
    

    //
    // Fonts
    // ---------------------
    //

    struct FontMetrics {
        int16_t units_per_em;
        int16_t ascender;
        int16_t descender;
        int16_t line_gap;
    };

    struct Font {
        FontMetrics metrics;

        const Glyph* glyphs;
        uint16_t glyph_count;
    };

} // namespace irvm

namespace irvm {
    constexpr uint8_t FORMAT_VERSION = 1;

    struct FileHeader {
        uint8_t  magic[5]; // "IRVM1"
        uint8_t  version;
        uint16_t width;
        uint16_t height;
        uint16_t object_count;
        uint8_t  palette_size;
        PaletteColor palette[MAX_PALETTE_SIZE];
    };

    //
    // Image set
    // -------------------------------------------------------------------------
    //
    // A single IRVM file can contain multiple vector objects.
    // All objects use the canvas dimensions from FileHeader.
    //

    struct ImageSet {
        FileHeader header;
        Palette palette;
        const VectorObject* objects;
        uint16_t object_count;
    };
}

namespace irvm {
    struct IndexedImage {
        uint16_t width;
        uint16_t height;
        std::vector<uint8_t> pixels;
    };

    /**
     * @brief Indexes an image with colors saved as singular bytes refering to color in a palette by index
     * 
     * @param origin original image
     * @param palette palette
     * @return IndexedImage 
     */
    IndexedImage index_image(const OriginImage& origin, const Palette& palette); // indexed.cpp
};

#endif // IRVM_IMAGE_HPP
