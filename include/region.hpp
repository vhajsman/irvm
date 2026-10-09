#ifndef IRVM_REGION_HPP
#define IRVM_REGION_HPP

#include "image.hpp"
#include <climits>
#include <cstdlib>
#include <optional>
#include <vector>

#ifndef IRVM_NEAR_INFINITE
#define IRVM_NEAR_INFINITE (INT_MAX - 1)
#endif

namespace irvm {
    struct RectUniformRegion {
        VectorCoordinate pos;
        VectorCoordinate dimensions;
        uint8_t color;
    };

    void merge_adjacent_rects(std::vector<RectUniformRegion>& rects);

    /**
     * @brief Check if rectangular area is uniform and return its color if so, otherwise return nullopt
     * 
     * @param indexed indexed image
     * @param rect rectangular area to be verified
     * @return std::optional<uint8_t> 
     */
    std::optional<uint8_t> uniform_color(const IndexedImage& indexed, const RectUniformRegion& rect);

    enum class CutAxis {
        vertical,
        horizontal
    };

    struct Cut {
        CutAxis axis;
        int position;
        int cost;
    };

    struct CutScore {
        int boundary_cost;      ///< boundary cost
        int non_uniform;        ///< non-uniform children
        int result_area_diff;   ///< resulting area difference;
    };

    /**
     * @brief Calculates cost of a vertical cut of uniform rectangular area
     * 
     * @param indexed indexed image
     * @param rect rectangular area to cut
     * @param cut_x count of pixels to cut
     * @return int 
     */
    int vertical_cut_cost(const IndexedImage& indexed, const RectUniformRegion& rect, int cut_x);
    
    /**
     * @brief Calculates cost of a horizontal cut of uniform rectangular area
     * 
     * @param indexed indexed image
     * @param rect rectangular area to cut
     * @param cut_y count of pixels to cut
     * @return int 
     */
    int horizontal_cut_cost(const IndexedImage& indexed, const RectUniformRegion& rect, int cut_y);

    /**
     * @brief Find best cut of cost as low as possible
     * 
     * @param indexed indexed image
     * @param rect rectangular area to cut
     *
     * @exception std::runtime_error
     *
     * @return Cut 
     */
    Cut find_best_cut(const IndexedImage& indexed, const RectUniformRegion& rect);

    /**
     * @brief Partitions the region into smaller ones by recursion
     * 
     * @param indexed indexed image
     * @param rect rectangular image to partition
     * @param out partitions vector
     *
     * @exception std::runtime_error
     */
    void rect_partition(const IndexedImage& indexed, const RectUniformRegion& rect, std::vector<RectUniformRegion>& out);
};

#endif
