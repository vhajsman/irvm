#include "origin.hpp"
#include "irvm_math.hpp"

#include <algorithm>    // for std::sort, std::min, std::max
#include <cmath>        // for std::pow, std::cbrt, std::sqrt
#include <iostream>
#include <limits.h>     // for UINT16_MAX
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace irvm {
    std::vector<ColorCluster> cluster_colors(const OriginImage& image, uint16_t cluster_count) {
        if(image.pixels.empty() || cluster_count == 0)
            return {};

        //
        // -------------------------------------------------------------
        // Build exact color histogram.
        // -------------------------------------------------------------
        //

        std::unordered_map<uint32_t, uint32_t> histogram;
        histogram.reserve(image.pixels.size());

        for(const OriginPixel& pixel : image.pixels) {
            const uint32_t key = color_key(pixel.r, pixel.g, pixel.b);
            ++histogram[key];
        }

        //
        // -------------------------------------------------------------
        // Convert histogram entries to samples.
        // -------------------------------------------------------------
        //

        std::vector<ColorSample> samples;
        samples.reserve(histogram.size());

        for(const auto& entry : histogram) {
            const uint32_t key = entry.first;

            const uint8_t r = static_cast<uint8_t>(key >> 16);
            const uint8_t g = static_cast<uint8_t>(key >> 8);
            const uint8_t b = static_cast<uint8_t>(key);

            ColorSample sample;
            sample.r = r;
            sample.g = g;
            sample.b = b;

            sample.count = entry.second;
            sample.lab = rgb_to_lab(r, g, b);

            samples.push_back(sample);
        }

        if(samples.size() <= cluster_count) {
            std::vector<ColorCluster> result;
            result.reserve(samples.size());

            for(const ColorSample& sample : samples) {
                result.push_back({
                    {
                        sample.r,
                        sample.g,
                        sample.b
                    },
                    sample.count
                });
            }

            std::sort(result.begin(), result.end(), [](const ColorCluster& lhs, const ColorCluster& rhs) {
                return lhs.count > rhs.count;
            });

            return result;
        }

        cluster_count = static_cast<uint16_t>(std::min<size_t>(cluster_count,samples.size()));

        //
        // -------------------------------------------------------------
        // Initial centers.
        // -------------------------------------------------------------
        //
        // Start with the most common color, then repeatedly select
        // the color furthest away from the existing centers.
        //

        std::sort(samples.begin(), samples.end(), [](const ColorSample& lhs, const ColorSample& rhs) {
            return lhs.count > rhs.count;
        });

        std::vector<Cluster> clusters;
        clusters.reserve(cluster_count);
        clusters.push_back({samples.front().lab, samples.front().count});

        std::vector<double> nearest_distance(samples.size(), std::numeric_limits<double>::max());
        for(uint16_t c = 1; c < cluster_count; ++c) {
            double best_score = -1.0;
            size_t best_index = 0;

            for(size_t i = 0; i < samples.size(); ++i) {
                const double distance = distance_squared(samples[i].lab, clusters.back().centroid);
                nearest_distance[i] = std::min(nearest_distance[i], distance);

                //
                // Farthest-point initialization.
                //
                // The first center is the most common color. Subsequent centers
                // are selected to be far from colors already represented.
                //
                // sqrt(count) gives frequently occurring colors more influence
                // without allowing a huge region to completely dominate the
                // diversity selection.
                //

                const double weight = std::sqrt(static_cast<double>(samples[i].count));
                const double score = nearest_distance[i] * weight;

                if(score > best_score) {
                    best_score = score;
                    best_index = i;
                }
            }

            clusters.push_back({samples[best_index].lab, 0});
        }

        //
        // -------------------------------------------------------------
        // Weighted k-means.
        // -------------------------------------------------------------
        //

        //
        // k-means normally continues until the centers stop moving.
        // Since this is an offline conversion step, we can afford several
        // iterations, but impose a hard limit so pathological input can
        // never cause an unbounded conversion.
        //
        // 32 iterations is normally more than enough for this small
        // clustering problem.
        //

        constexpr unsigned MAX_ITERATIONS = 32;

        std::vector<uint16_t> assignments(samples.size(), std::numeric_limits<uint16_t>::max());
        for(unsigned iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {
            bool changed = false;

            //
            // Assignment step.
            //

            for(size_t i = 0; i < samples.size(); ++i) {
                double best_distance = std::numeric_limits<double>::max();
                uint16_t best_cluster = 0;

                for(uint16_t c = 0; c < clusters.size(); ++c) {
                    const double distance = distance_squared(samples[i].lab, clusters[c].centroid);

                    if(distance < best_distance) {
                        best_distance = distance;
                        best_cluster = c;
                    }
                }

                if(assignments[i] != best_cluster)
                    changed = true;

                assignments[i] = best_cluster;
            }

            //
            // Recalculate centers.
            //

            std::vector<double> sum_l(clusters.size(), 0.0);
            std::vector<double> sum_a(clusters.size(), 0.0);
            std::vector<double> sum_b(clusters.size(), 0.0);
            
            std::vector<uint64_t> counts(clusters.size(), 0);

            for(size_t i = 0; i < samples.size(); ++i) {
                const uint16_t cluster = assignments[i];
                const double weight = static_cast<double>( samples[i].count);

                sum_l[cluster] += samples[i].lab.l * weight;
                sum_a[cluster] += samples[i].lab.a * weight;
                sum_b[cluster] += samples[i].lab.b * weight;

                counts[cluster] += samples[i].count;
            }

            double movement = 0.0;

            for(size_t c = 0; c < clusters.size(); ++c) {
                if(counts[c] == 0)
                    continue;

                Lab new_center;
                new_center.l = sum_l[c] / counts[c];
                new_center.a = sum_a[c] / counts[c];
                new_center.b = sum_b[c] / counts[c];

                movement += distance_squared(clusters[c].centroid, new_center);

                clusters[c].centroid = new_center;
                clusters[c].pixel_count = counts[c];
            }

            if(!changed || movement < 0.001)
                break;
        }

        //
        // -------------------------------------------------------------
        // Convert clusters back to RGB.
        // -------------------------------------------------------------
        //

        std::vector<ColorCluster> result;
        result.reserve(clusters.size());

        unsigned int verify = 0;
        for(const Cluster& cluster : clusters) {
            if(cluster.pixel_count == 0)
                continue;

            const ClusterColor color = lab_to_rgb(cluster.centroid);
            result.push_back({color, static_cast<uint32_t>(cluster.pixel_count)});

            verify += cluster_count;

            //std::cout << "Cluster: RGB(" << static_cast<int>(color.r) << ", " << static_cast<int>(color.g) << ", " << static_cast<int>(color.b) << ") Count: " << cluster.pixel_count << "\n";
        }

        if(verify != image.width * image.height) {
            throw std::runtime_error("Cluser algorithm verification failed");
        }

        std::sort(result.begin(), result.end(), [](const ColorCluster& lhs, const ColorCluster& rhs) {
            return lhs.count > rhs.count;
        });

        return result;
    }
};
