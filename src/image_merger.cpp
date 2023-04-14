//
// Created by nikola on 4/7/23.
//

#include <iostream>
#include <algorithm>
#include <span>
#include <numeric>
#include <random>
#include "image_merger.h"

std::filesystem::path
ImageMerger::merge_images(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                          const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            auto out_header = first_image.getHeader();
            auto const &first_vector = first_image.getPixelData();
            auto const &second_vector = second_image.getPixelData();
            size_t const height = first_image.getHeader().height;
            size_t const width = first_vector.size() / height;

            auto first_array = get_2d_pixels(first_vector, height);
            auto second_array = get_2d_pixels(second_vector, height);

            std::vector<std::vector<std::byte>> out_array(height, std::vector<std::byte>(width));


            for (int j = 0; j < width; ++j) {
                for (int i = 0; i < height; ++i) {
                    std::byte pixel{};
                    if (merger) {
                        pixel = std::byte(weight * std::to_integer<int>(first_array[i][j]) +
                                          (1 - weight) * std::to_integer<int>(second_array[i][j]));

                    } else {
                        pixel = std::byte(std::max(std::to_integer<int>(first_array[i][j]),
                                                   std::to_integer<int>(second_array[i][j])));
                    }
                    out_array[i][j] = pixel;
                }
            }

            auto out_vec = get_vec_pixels(out_array);


            Bmp out{out_header, out_vec};
            return absolute(out.write_image(out_path));
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}


std::filesystem::path
ImageMerger::merge_images_cache(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                                const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            // cast vector to span
            auto const &first_pixel_data = std::span<const std::byte>(first_image.getPixelData());
            auto const &second_pixel_data = std::span<const std::byte>(second_image.getPixelData());

            auto out_header = first_image.getHeader();
            std::vector<std::byte> out_pixels(first_pixel_data.size());

            for (size_t i = 0; i < first_pixel_data.size(); ++i) {
                //change of access at() -> []

                std::byte pixel{};
                if (merger) {
                    pixel = std::byte(weight * std::to_integer<int>(first_pixel_data[i]) +
                                      (1 - weight) * std::to_integer<int>(second_pixel_data[i]));
                } else {
                    pixel = std::byte(std::max(std::to_integer<int>(first_pixel_data[i]),
                                               std::to_integer<int>(second_pixel_data[i])));
                }
                out_pixels[i] = pixel;
            }

            Bmp out{out_header, out_pixels};
            return absolute(out.write_image(out_path));

        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}


std::filesystem::path
ImageMerger::merge_images_openmp(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                                 const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            auto out_header = first_image.getHeader();
            size_t const height = out_header.height;

            auto const &first_pixel_data = get_2d_pixels(first_image.getPixelData(), height);
            auto const &second_pixel_data = get_2d_pixels(second_image.getPixelData(), height);
            size_t const width = first_pixel_data[0].size();
            std::vector<std::vector<std::byte>> out_array(height, std::vector<std::byte>(width));


#pragma omp parallel default(shared)
            {
#pragma omp for
                for (size_t i = 0; i < height; ++i) {
#pragma omp parallel shared(i, height)
                    {
#pragma omp for
                        for (size_t j = 0; j < width; ++j) {
                            std::byte pixel{};
                            if (merger) {
                                pixel = std::byte(weight * std::to_integer<int>(first_pixel_data[i][j]) +
                                                  (1 - weight) * std::to_integer<int>(second_pixel_data[i][j]));

                            } else {
                                pixel = std::byte(std::max(std::to_integer<int>(first_pixel_data[i][j]),
                                                           std::to_integer<int>(second_pixel_data[i][j])));
                            }
                            out_array[i][j] = pixel;
                        }
                    }
                }

            }
            auto out_vec = get_vec_pixels(out_array);

            Bmp out{out_header, out_vec};
            return absolute(out.write_image(out_path));

        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}


std::filesystem::path
ImageMerger::merge_images_optimized(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
                                    const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            auto const &first_pixel_data = first_image.getPixelData();
            auto const &second_pixel_data = second_image.getPixelData();

            auto out_header = first_image.getHeader();
            std::vector<std::byte> out_pixels(first_pixel_data.size());

#pragma omp parallel for

            for (size_t i = 0; i < first_pixel_data.size(); ++i) {
                //change of access at() -> []
                std::byte pixel{};
                if (merger) {
                    pixel = std::byte(weight * std::to_integer<int>(first_pixel_data[i]) +
                                      (1 - weight) * std::to_integer<int>(second_pixel_data[i]));
                } else {
                    pixel = std::byte(std::max(std::to_integer<int>(first_pixel_data[i]),
                                               std::to_integer<int>(second_pixel_data[i])));
                }
                out_pixels[i] = pixel;
            }

            Bmp out{out_header, out_pixels};
            return absolute(out.write_image(out_path));

        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}

std::vector<std::vector<std::byte>> ImageMerger::get_2d_pixels(const std::vector<std::byte> &pixels, int height) {

    int const width = pixels.size() / height;
    std::vector<std::vector<std::byte>> ret(height, std::vector<std::byte>(width));
#pragma omp parallel for
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            ret[i][j] = pixels[i * width + j];
        }
    }

    return ret;
}

std::vector<std::byte> ImageMerger::get_vec_pixels(const std::vector<std::vector<std::byte>> &pixels) {
    std::size_t height = pixels.size();
    std::size_t width = pixels[0].size();
    std::vector<std::byte> ret(width * height);
#pragma omp parallel default(shared)
    {
#pragma omp for
        for (size_t i = 0; i < height; ++i) {
#pragma omp parallel shared(i, height)
            {
#pragma omp for
                for (size_t j = 0; j < width; ++j) {
                    ret[i * width + j] = pixels[i][j];
                }
            }
        }
    }

    return ret;

}
