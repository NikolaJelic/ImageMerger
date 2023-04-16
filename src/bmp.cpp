//
// Created by nikola on 4/7/23.
//

#include <iostream>
#include <span>
#include "bmp.h"


std::vector<std::byte> Bmp::load_image(const std::filesystem::path &image_path) {
    if (is_regular_file(image_path)) {
        auto image = std::ifstream{image_path, std::ios::binary | std::ios::ate};
        if (!image) { return {}; }
        auto const size = image.tellg();
        image.seekg({}, std::ios::beg);
        std::vector<std::byte> ret(size);
        image.read(reinterpret_cast<char *>(ret.data()), size);
        return ret;
    }
    return {};
}

Bmp::Bmp(const std::filesystem::path &image_path) {
    try {
        if (image_path.extension() != ".bmp") {
            throw std::runtime_error("Invalid file type.\n");

        }
        auto image = load_image(image_path);
        if (!image.empty()) {
            std::copy(image.begin(), image.begin() + header_size, reinterpret_cast<std::byte *>(&header));
            pixel_data.resize(image.size() - header.offset);
            std::copy(image.begin() + header.offset, image.end(), pixel_data.data());
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

}

const Bmp::BmpHeader &Bmp::getHeader() const {
    return header;
}


const std::vector<std::byte> &Bmp::getPixelData() const {
    return pixel_data;
}


Bmp::Bmp(const Bmp::BmpHeader &header, const std::vector<std::byte> &pixelData) : header(header),
                                                                                  pixel_data(pixelData) {}

std::filesystem::path Bmp::write_image(const std::filesystem::path &path) {
    try {
        if (path.extension() == ".bmp") {
            std::ofstream out{path, std::ios::binary};
            if (out.is_open()) {
                out.write(reinterpret_cast<char *>(&header), sizeof(header));
                std::vector<std::byte> offset_fill(header.offset - sizeof(header));
                std::fill(offset_fill.begin(), offset_fill.end(), std::byte{0x00});
                out.write(reinterpret_cast<char *>(offset_fill.data()), header.offset - sizeof(header));
                auto pixels = std::span<const std::byte>(std::move(pixel_data));
                out.write(reinterpret_cast<const char *>(pixels.data()), pixels.size());
                return path;
            } else {
                throw std::runtime_error("File " + path.string() + " failed to open.\n");
            }
        } else {
            throw std::runtime_error("File " + path.string() + " is not a .bmp file.\n");
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return {};
    }
}
