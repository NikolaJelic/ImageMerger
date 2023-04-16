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
        [[maybe_unused]] uint16_t signature;
        [[maybe_unused]] uint32_t file_size;
        [[maybe_unused]] uint16_t reserved1;
        [[maybe_unused]] uint16_t reserved2;
        uint32_t offset;
        [[maybe_unused]] uint32_t header_size;
        int32_t width;
        int32_t height;
        [[maybe_unused]] uint16_t planes;
        [[maybe_unused]] uint16_t bits_per_pixel;
        [[maybe_unused]] uint32_t compression;
        [[maybe_unused]] uint32_t image_size;
        [[maybe_unused]] int32_t x_pixels_per_meter;
        [[maybe_unused]] int32_t y_pixels_per_meter;
        [[maybe_unused]] uint32_t colors_used;
        [[maybe_unused]] uint32_t important_colors;
    };
    const std::size_t header_size = 54;
    BmpHeader header{};
    std::vector<std::byte> pixel_data{};

#pragma pack(pop)

/**
 * Load an image from a file path into a vector of bytes.
 *
 * @param image_path The path of the image file to load.
 * @return A vector of bytes representing the loaded image, or an empty vector if loading failed.
 */
    std::vector<std::byte> load_image(std::filesystem::path const &image_path);

public:
    /**
 * Constructs a BMP object by loading an image from a file.
 *
 * @param image_path The path of the BMP image file to load.
 */
    explicit Bmp(std::filesystem::path const &image_path);

    /**
 * Constructs a BMP object with the given BMP header and pixel data.
 *
 * @param header The BMP header data.
 * @param pixelData The pixel data.
 */
    Bmp(const BmpHeader &header, const std::vector<std::byte> &pixelData);

    /**
 * Gets the BMP header data of the loaded image.
 *
 * @return A reference to the BMP header data.
 */
    [[nodiscard]] const BmpHeader &getHeader() const;

/**
 * Gets the pixel data of the loaded image.
 *
 * @return A reference to the vector of bytes representing the pixel data.
 */
    [[nodiscard]] const std::vector<std::byte> &getPixelData() const;

    /**
 * Writes the BMP image to a file at the given path.
 *
 * @param path The path of the file to write the BMP image to.
 * @return The path of the file that was written to, or an empty path if writing failed.
 */
    std::filesystem::path write_image(std::filesystem::path const &path);
};
