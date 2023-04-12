//
// Created by nikola on 4/7/23.
//

#include <iostream>
#include <algorithm>
#include <span>
#include <numeric>
#include <random>
#include "image_merger.h"

const std::vector<std::byte> &
ImageMerger::merge_images(int merger, std::filesystem::path &first, const std::filesystem::path &second,
                          const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            auto out_header = first_image.getHeader();
            auto const& first_vector = first_image.getPixelData();
            auto const& second_vector = second_image.getPixelData();
            int const height = first_image.getHeader().height;
            int const width  = first_vector.size()/height;

            std::cout << "header loaded\n";
            std::vector<std::vector<std::byte>> first_array (height, std::vector<std::byte>(width));
            std::vector<std::vector<std::byte>> second_array  (height, std::vector<std::byte>(width));
            std::vector<std::vector<std::byte>> out_array  (height, std::vector<std::byte>(width));


            std::cout << "vec: " << first_vector.size();

            for(int i = 0; i < height; ++i){
                for(int j = 0; j <  width; ++j){
                    first_array[i][j] = first_vector[ i * width + j];
                    second_array[i][j] = second_vector[i * width + j];
                }
            }

            std::cout << "array made\n";



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

            std::vector<std::byte> out_vec(height * width);
            std::cout <<  out_vec.size() << "  " << first_vector.size() << '\n';

            for(int i = 0; i < height; ++i){
                for(int j = 0; j <  width; ++j){
                    out_vec[i * width + j] = out_array[i][j];
                }
            }


            Bmp out{out_header, out_vec};
            std::string ret = out.write_image(out_path);
            return out.getPixelData();
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}


const std::vector<std::byte> &
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

            for (int i = 0; i < first_pixel_data.size(); ++i) {
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
            std::string ret = std::move(out.write_image(out_path));
            return out.getPixelData();
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}


const std::vector<std::byte> &
ImageMerger::merge_images_openmp(int merger, const std::filesystem::path &first, const std::filesystem::path &second,
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

            for (int i = 0; i < first_pixel_data.size(); ++i) {
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
            std::string ret = std::move(out.write_image(out_path));
            return out.getPixelData();
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}


const std::vector<std::byte> &
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

            for (int i = 0; i < first_pixel_data.size(); ++i) {
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
            std::string ret = std::move(out.write_image(out_path));
            return out.getPixelData();
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}
