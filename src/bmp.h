#pragma once

#include <fstream>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <filesystem>
#include <omp.h>

class Bmp {
#pragma pack(push, 1)
    struct BmpHeader {
        uint16_t signature;
        uint32_t file_size;
        uint16_t reserved1;
        uint16_t reserved2;
        uint32_t offset;
        uint32_t header_size;
        int32_t width;
        int32_t height;
        uint16_t planes;
        uint16_t bits_per_pixel;
        uint32_t compression;
        uint32_t image_size;
        int32_t x_pixels_per_meter;
        int32_t y_pixels_per_meter;
        uint32_t colors_used;
        uint32_t important_colors;
    };
    const std::size_t header_size = 54;
    BmpHeader header{};
    std::vector<std::byte> pixel_data{};

#pragma pack(pop)
    std::vector<std::byte> load_image(std::filesystem::path const &image_path);

public:
    explicit Bmp(std::filesystem::path const &image_path);
    Bmp(const BmpHeader &header, const std::vector<std::byte> &pixelData);

    [[nodiscard]] const BmpHeader &getHeader() const;
    void setHeader(const BmpHeader &header);

    [[nodiscard]] const std::vector<std::byte> &getPixelData() const;
    void setPixelData(const std::vector<std::byte> &pixelData);

    [[nodiscard]] std::vector<std::vector<std::byte>>get_2d_pixel_data()const;

    std::filesystem::path write_image(std::filesystem::path const& path);

    void set_2d_pixel_data(const std::vector<std::vector<std::byte>> &data);
};
