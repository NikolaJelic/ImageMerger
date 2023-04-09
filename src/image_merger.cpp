//
// Created by nikola on 4/7/23.
//

#include <iostream>
#include <algorithm>
#include <span>
#include "image_merger.h"


std::filesystem::path ImageMerger::merge_images(const std::filesystem::path &first, const std::filesystem::path &second,
                                                const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            auto out_header = first_image.getHeader();
            std::vector<std::byte> out_pixels{};
            for (int i = 0; i < first_image.getPixelData().size(); ++i) {
                auto pixel = std::byte(weight * std::to_integer<int>(first_image.getPixelData().at(i)) +
                                       (1 - weight) * std::to_integer<int>(second_image.getPixelData().at(i)));
                out_pixels.push_back(pixel);
            }
            Bmp out{out_header, out_pixels};
            std::string ret = out.write_image(out_path);
            return ret;
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}

std::filesystem::path
ImageMerger::merge_images_cache(const std::filesystem::path &first, const std::filesystem::path &second,
                                const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            // cast vector to span
            auto const& first_pixel_data = std::span<const std::byte>(first_image.getPixelData());
            auto const& second_pixel_data = std::span<const std::byte>(second_image.getPixelData());

            auto out_header = first_image.getHeader();
            std::vector<std::byte> out_pixels(first_pixel_data.size());
            std::transform(first_pixel_data.begin(), first_pixel_data.end(),
                           second_pixel_data.begin(), out_pixels.begin(),
                           [weight](std::byte first_pixel, std::byte second_pixel) {
                               return std::byte(weight * std::to_integer<int>(first_pixel) +
                                                (1 - weight) * std::to_integer<int>(second_pixel));
                           });
            Bmp out{out_header, out_pixels};
            std::string ret = std::move(out.write_image(out_path));
            return ret;
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};
}


std::filesystem::path
ImageMerger::merge_images_openmp(const std::filesystem::path &first, const std::filesystem::path &second,
                                 const std::filesystem::path &out_path, float weight) {
    try {
        Bmp first_image{first};
        Bmp second_image{second};

        if (first_image.getHeader().height != second_image.getHeader().height ||
            first_image.getHeader().width != second_image.getHeader().width) {
            throw std::runtime_error("Images aren't matching.\n");
        } else {
            auto const& first_pixel_data = first_image.getPixelData();
            auto const& second_pixel_data = second_image.getPixelData();

            auto out_header = first_image.getHeader();
            std::vector<std::byte> out_pixels(first_pixel_data.size());
            #pragma omp parallel for
            for (int i = 0; i < first_image.getPixelData().size(); ++i) {
                //change of access at() -> []
                auto pixel = std::byte(weight * std::to_integer<int>(first_pixel_data[i]) +
                                       (1 - weight) * std::to_integer<int>(second_pixel_data[i]));
                out_pixels[i] = pixel;
            }
            Bmp out{out_header, out_pixels};
            std::string ret = std::move(out.write_image(out_path));
            return ret;
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
    return {};}

std::filesystem::path
ImageMerger::merge_images_optimized(const std::filesystem::path &first, const std::filesystem::path &second,
                                    const std::filesystem::path &out_path, float weight) {
    return std::filesystem::path();
}
