#include "region.hpp"
#include "image.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

namespace irvm {
    struct RectBounds {
        int min_x = 0;
        int max_x = 0;
        int min_y = 0;
        int max_y = 0;
        bool valid = false;
    };

    static RectUniformRegion h_make_first_rect(const RectUniformRegion& rect, const Cut& cut);
    static RectUniformRegion h_make_second_rect(const RectUniformRegion& rect, const Cut& cut);

    static bool h_better_cut_score(const CutScore& candidate, const CutScore& best);

    static bool h_merge_horizontal(std::vector<RectUniformRegion>& rects);
    static bool h_merge_vertical(std::vector<RectUniformRegion>& rects);

    // static RectBounds h_find_non_bg_bounds(const IndexedImage& indexed, uint8_t background);

    /**
     * @brief Make the first rectangle after a cut.
     */
    static RectUniformRegion h_make_first_rect(const RectUniformRegion& rect, const Cut& cut) {
        RectUniformRegion result;
        result.pos = rect.pos;
        result.color = rect.color;

        if(cut.axis == CutAxis::vertical) {
            result.dimensions.x = static_cast<int16_t>(cut.position - rect.pos.x);
            result.dimensions.y = rect.dimensions.y;
        } else {
            result.dimensions.x = rect.dimensions.x;
            result.dimensions.y = static_cast<int16_t>(cut.position - rect.pos.y);
        }

        return result;
    }

    /**
     * @brief Make the second rectangle after a cut.
     */
    static RectUniformRegion h_make_second_rect(const RectUniformRegion& rect, const Cut& cut) {
        RectUniformRegion result;
        result.color = rect.color;

        if(cut.axis == CutAxis::vertical) {
            result.pos.x = static_cast<int16_t>(cut.position);
            result.pos.y = rect.pos.y;

            result.dimensions.x = static_cast<int16_t>(rect.pos.x + rect.dimensions.x - cut.position);
            result.dimensions.y = rect.dimensions.y;
        } else {
            result.pos.x = rect.pos.x;
            result.pos.y = static_cast<int16_t>(cut.position);

            result.dimensions.x = rect.dimensions.x;
            result.dimensions.y = static_cast<int16_t>(rect.pos.y + rect.dimensions.y - cut.position);
        }

        return result;
    }

    /**
     * @brief Check if candidate has better score than best cut yet
     * 
     * @param candidate candidate score
     * @param best yet best cut score
     *
     * @return true 
     * @return false 
     */
    static bool h_better_cut_score(const CutScore& candidate, const CutScore& best) {
        if(candidate.non_uniform != best.non_uniform)
            return candidate.non_uniform < best.non_uniform;
        if(candidate.boundary_cost != best.boundary_cost)
            return candidate.boundary_cost < best.boundary_cost;
        return candidate.result_area_diff < best.result_area_diff;
    }

    /**
     * @brief 
     * 
     * @param rects 
     * @return true 
     * @return false 
     */
    static bool h_merge_horizontal(std::vector<RectUniformRegion>& rects) {
        std::sort(rects.begin(), rects.end(), [](const auto& a, const auto& b) {
            if(a.color != b.color) return a.color < b.color;
            if(a.pos.y != b.pos.y) return a.pos.y < b.pos.y;
            
            if(a.dimensions.y != b.dimensions.y)
                return a.dimensions.y < b.dimensions.y;

            return a.pos.x < b.pos.x;
        });

        std::vector<RectUniformRegion> merged;
        merged.reserve(rects.size());

        for(const auto& rect : rects) {
            if(!merged.empty()) {
                auto& prev = merged.back();

                if(prev.color == rect.color && prev.pos.y == rect.pos.y && prev.dimensions.y == rect.dimensions.y && prev.pos.x + prev.dimensions.x == rect.pos.x) {
                    prev.dimensions.x = static_cast<int16_t>(prev.dimensions.x + rect.dimensions.x);
                    continue;
                }
            }

            merged.push_back(rect);
        }

        const bool changed = merged.size() != rects.size();
        rects = std::move(merged);

        return changed;
    }

    /**
     * @brief 
     * 
     * @param rects 
     * @return true 
     * @return false 
     */
    static bool h_merge_vertical(std::vector<RectUniformRegion>& rects) {
        std::sort(rects.begin(), rects.end(), [](const auto& a, const auto& b) {
            if(a.color != b.color) return a.color < b.color;
            if(a.pos.x != b.pos.x) return a.pos.x < b.pos.x;
            
            if(a.dimensions.x != b.dimensions.x)
                return a.dimensions.x < b.dimensions.x;

            return a.pos.y < b.pos.y;
        });

        std::vector<RectUniformRegion> merged;
        merged.reserve(rects.size());

        for(const auto& rect : rects) {
            if(!merged.empty()) {
                auto& prev = merged.back();

                if(prev.color == rect.color && prev.pos.x == rect.pos.x && prev.dimensions.x == rect.dimensions.x && prev.pos.y + prev.dimensions.y == rect.pos.y) {
                    prev.dimensions.y = static_cast<int16_t>(prev.dimensions.y + rect.dimensions.y);
                    continue;
                }
            }

            merged.push_back(rect);
        }

        const bool changed = merged.size() != rects.size();
        rects = std::move(merged);

        return changed;
    }

    void merge_adjacent_rects(std::vector<RectUniformRegion>& rects) {
        bool changed;

        do {
            changed = h_merge_horizontal(rects);
            changed = h_merge_vertical(rects) || changed;
        } while(changed);
    }

//    /**
//     * @brief Find the bounding rectangle of all non-background pixels.
//     */
//    static RectBounds h_find_non_bg_bounds(const IndexedImage& indexed, uint8_t background) {
//        RectBounds bounds;
//
//        for(int y = 0; y < indexed.height; ++y) {
//            for(int x = 0; x < indexed.width; ++x) {
//                const size_t idx = static_cast<size_t>(y) * indexed.width + x;
//
//                if(indexed.pixels[idx] == background)
//                    continue;
//
//                if(!bounds.valid) {
//                    bounds.min_x = bounds.max_x = x;
//                    bounds.min_y = bounds.max_y = y;
//                    bounds.valid = true;
//
//                    continue;
//                }
//
//                bounds.min_x = std::min(bounds.min_x, x);
//                bounds.max_x = std::max(bounds.max_x, x);
//                bounds.min_y = std::min(bounds.min_y, y);
//                bounds.max_y = std::max(bounds.max_y, y);
//            }
//        }
//
//        return bounds;
//    }

    std::optional<uint8_t> uniform_color(const IndexedImage& indexed, const RectUniformRegion& rect) {
        if(rect.dimensions.x <= 0 || rect.dimensions.y <= 0)
            return std::nullopt;

        const size_t start = static_cast<size_t>(rect.pos.y) * indexed.width + rect.pos.x;
        const uint8_t color = indexed.pixels[start];

        for(int y = rect.pos.y; y < rect.pos.y + rect.dimensions.y; ++y) {
            for(int x = rect.pos.x; x < rect.pos.x + rect.dimensions.x; ++x) {
                const size_t idx = static_cast<size_t>(y) * indexed.width + x;

                if(indexed.pixels[idx] != color)
                    return std::nullopt;
            }
        }

        return color;
    }

    int vertical_cut_cost(const IndexedImage& indexed, const RectUniformRegion& rect, int cut_x) {
        int cost = 0;

        for(int y = rect.pos.y; y < rect.pos.y + rect.dimensions.y; ++y) {
            const size_t idx_l = static_cast<size_t>(y) * indexed.width + cut_x - 1;
            const size_t idx_r = static_cast<size_t>(y) * indexed.width + cut_x;

            if(indexed.pixels[idx_l] != indexed.pixels[idx_r])
                ++cost;
        }

        return cost;
    }

    int horizontal_cut_cost(const IndexedImage& indexed, const RectUniformRegion& rect, int cut_y) {
        int cost = 0;

        for(int x = rect.pos.x; x < rect.pos.x + rect.dimensions.x; ++x) {
            const size_t idx_t = static_cast<size_t>(cut_y - 1) * indexed.width + x;
            const size_t idx_b = static_cast<size_t>(cut_y) * indexed.width + x;

            if(indexed.pixels[idx_t] != indexed.pixels[idx_b])
                ++cost;
        }

        return cost;
    }

    Cut find_best_cut(const IndexedImage& indexed, const RectUniformRegion& rect) {
        Cut best;
        best.axis = CutAxis::vertical;
        best.cost = IRVM_NEAR_INFINITE;
        best.position = 0;

        CutScore score_best {
            IRVM_NEAR_INFINITE,
            IRVM_NEAR_INFINITE,
            IRVM_NEAR_INFINITE
        };

        bool found = false;

        auto evaluate = [&](CutAxis axis, int pos) {
            Cut candidate;
            candidate.axis = axis;
            candidate.position = pos;
            candidate.cost = 0;

            const RectUniformRegion first = h_make_first_rect(rect, candidate);
            const RectUniformRegion second = h_make_second_rect(rect, candidate);

            const int non_uniform = 
                static_cast<int>(!uniform_color(indexed, first).has_value()) + 
                static_cast<int>(!uniform_color(indexed, second).has_value());
            
            const int boundary_cost = axis == CutAxis::vertical
                ? vertical_cut_cost(indexed, rect, pos) 
                : horizontal_cut_cost(indexed, rect, pos);

            const int first_area = first.dimensions.x * first.dimensions.y;
            const int second_area = second.dimensions.x * second.dimensions.y;

            const CutScore score {
                boundary_cost,
                non_uniform,
                std::abs(first_area - second_area)
            };

            if(!found || h_better_cut_score(score, score_best)) {
                best = {
                    axis,
                    pos,
                    boundary_cost
                };

                score_best = score;
                found = true;
            }
        };

        // Evaluate vertical cuts.
        for(int x = rect.pos.x + 1; x < rect.pos.x + rect.dimensions.x; ++x)
            evaluate(CutAxis::vertical, x);

        // Evaluate horizontal cuts.
        for(int y = rect.pos.y + 1; y < rect.pos.y + rect.dimensions.y; ++y)
            evaluate(CutAxis::horizontal, y);

        if(!found)
            throw std::runtime_error("Failed to cut rectangular region");

        return best;
    }

    void rect_partition(const IndexedImage& indexed, const RectUniformRegion& rect, std::vector<RectUniformRegion>& out) {
        const auto color = uniform_color(indexed, rect);

        if(color.has_value()) {
            out.push_back({
                rect.pos,
                rect.dimensions,
                color.value()
            });

            return;
        }

        // A single pixel is always uniform.
        if(rect.dimensions.x == 1 && rect.dimensions.y == 1) {
            const size_t index = static_cast<size_t>(rect.pos.y) * indexed.width + rect.pos.x;

            out.push_back({
                rect.pos,
                {1, 1},
                indexed.pixels[index]
            });

            return;
        }

        const Cut cut = find_best_cut(indexed, rect);

        const RectUniformRegion first = h_make_first_rect(rect, cut);
        const RectUniformRegion second = h_make_second_rect(rect, cut);

        rect_partition(indexed, first, out);
        rect_partition(indexed, second, out);
    }

    void sort_rectangles(std::vector<RectUniformRegion>& rectangles, SortMode sort_mode) {
        if(sort_mode == SortMode::position) {
            std::sort(rectangles.begin(), rectangles.end(), [](const auto& a, const auto& b) {
                if(a.pos.y != b.pos.y) return a.pos.y < b.pos.y;
                if(a.pos.x != b.pos.x) return a.pos.x < b.pos.x;

                return a.color < b.color;
            });
        } else {
            std::sort(rectangles.begin(), rectangles.end(), [](const auto& a, const auto& b) {
                if(a.color != b.color) return a.color < b.color;
                if(a.pos.y != b.pos.y) return a.pos.y < b.pos.y;

                return a.pos.x < b.pos.x;
            });
        }
    }
}