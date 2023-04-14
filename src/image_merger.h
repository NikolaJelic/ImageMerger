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

    std::vector<std::vector<std::byte>> get_2d_pixels(std::vector<std::byte> const &pixels, int height);

    std::vector<std::byte> get_vec_pixels(std::vector<std::vector<std::byte>> const &pixels);

public:
/**
 * Merges two images into a single image using either weighted or non-weighted blending.
 *
 * @param merger    An integer indicating the type of blending. A value of 0 corresponds to non-weighted blending,
 *                      while a value of 1 corresponds to weighted blending.
 * @param first     The path to the first image to merge.
 * @param second    The path to the second image to merge.
 * @param out_path  The path where the merged image will be written.
 * @param weight    A float value that determines the blending ratio when weighted blending is used.
 *
 * @return             The absolute path to the merged image file.
 */
    std::filesystem::path
    merge_images(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                 const std::filesystem::path &out_path, float weight = 0.5f);

/**
 * Merges two images into a single image using either weighted or non-weighted blending,
 * using span instead of vectors to store pixel data and iterating through a 1D array instead of a 2D matrix to improve cache performance.
 *
 * @param merger    An integer indicating the type of blending. A value of 0 corresponds to non-weighted blending,
 *                      while a value of 1 corresponds to weighted blending.
 * @param first     The path to the first image to merge.
 * @param second    The path to the second image to merge.
 * @param out_path  The path where the merged image will be written.
 * @param weight    A float value that determines the blending ratio when weighted blending is used.
 *
 * @return             The absolute path to the merged image file.
 */
    std::filesystem::path
    merge_images_cache(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                       const std::filesystem::path &out_path, float weight = 0.5f);

/**
 * Merges two images into a single image using either weighted or non-weighted blending,
 * using OpenMP for parallelization.
 *
 * @param merger    An integer indicating the type of blending. A value of 0 corresponds to non-weighted blending,
 *                      while a value of 1 corresponds to weighted blending.
 * @param first     The path to the first image to merge.
 * @param second    The path to the second image to merge.
 * @param out_path  The path where the merged image will be written.
 * @param weight    A float value that determines the blending ratio when weighted blending is used.
 *
 * @return             The absolute path to the merged image file.
 */
    std::filesystem::path
    merge_images_openmp(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                        const std::filesystem::path &out_path, float weight = 0.5f);

/**
 * Merges two images into a single image using either weighted or non-weighted blending,
 * using OpenMP for parallelization and a 1D vector for cache optimization.
 *
 * @param merger    An integer indicating the type of blending. A value of 0 corresponds to non-weighted blending,
 *                      while a value of 1 corresponds to weighted blending.
 * @param first     The path to the first image to merge.
 * @param second    The path to the second image to merge.
 * @param out_path  The path where the merged image will be written.
 * @param weight    A float value that determines the blending ratio when weighted blending is used.
 *
 * @return             The absolute path to the merged image file.
 */
    std::filesystem::path
    merge_images_optimized(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                           const std::filesystem::path &out_path, float weight);
};
