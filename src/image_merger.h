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


class ImageMerger{

    const std::size_t header_size = 54;
    const std::size_t pixel_data_offset_index = 10;
    const std::size_t file_size_index = 2;
    const std::size_t height_index = 22;
    const std::size_t width_index = 18;
    const std::size_t bits_per_pixel = 28;

    std::filesystem::path first_image_path{};
    std::filesystem::path second_image_path{};
    ///[height, width]
    std::pair<std::uint32_t , std::uint32_t> image_dimensions{};

public:
    std::filesystem::path merge_images(const std::filesystem::path &first, const std::filesystem::path &second,
                                       const std::filesystem::path &out_path, float weight = 0.5f);
    std::filesystem::path merge_images_cache(const std::filesystem::path &first, const std::filesystem::path &second,
                                       const std::filesystem::path &out_path, float weight = 0.5f);
    std::filesystem::path merge_images_openmp(const std::filesystem::path &first, const std::filesystem::path &second,
                                       const std::filesystem::path &out_path, float weight = 0.5f);

    std::filesystem::path merge_images_optimized(const std::filesystem::path &first, const std::filesystem::path &second,
                                       const std::filesystem::path &out_path, float weight = 0.5f);
};
