//
// Created by nikola on 4/7/23.
//

#pragma once


#include <cstddef>
#include <filesystem>
#include <vector>
#include <fstream>
#include "bmp.h"
#include <omp.h>
#include <functional>


class ImageMerger {

public:
    /// merger value: 0=max, 1=average
    const std::vector<std::byte> &
    merge_images(int merger, std::filesystem::path &first, const std::filesystem::path &second,
                 const std::filesystem::path &out_path, float weight = 0.5f);

    /// merger value: 0=max, 1=average
    const std::vector<std::byte> &
    merge_images_cache(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                       const std::filesystem::path &out_path, float weight = 0.5f);

    /// merger value: 0=max, 1=average
    const std::vector<std::byte> &
    merge_images_openmp(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                        const std::filesystem::path &out_path, float weight = 0.5f);

    const std::vector<std::byte> &
    merge_images_optimized(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                           const std::filesystem::path &out_path, float weight);
};
