//
// Created by nikola on 4/7/23.
//

#include <iostream>
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
            return out.write_image(out_path);
        }
    } catch (std::exception &e) { std::cerr << e.what(); }
}
