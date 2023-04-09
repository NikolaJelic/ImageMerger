#include <iostream>
#include "image_merger.h"
#include "bmp.h"

int main(int argc, char *argv[]) {
    if (argc == 2 && std::string(argv[1]) == "help") {
        std::cout << "Correct input: " << argv[0]
                  << " <algorithm version (base, cache, opencl, optimized)> <path to first image> <path to second image> <path to output> <weight> "
                  << std::endl;
        std::cout << "Arguments:\n"
                  << "  algorithm version: the version of the algorithm to use (base, cache, opencl, optimized)\n"
                  << "  path to first image: the path to the first input image file\n"
                  << "  path to second image: the path to the second input image file\n"
                  << "  path to output: the path to the output image file\n"
                  << "  weight: the weight to use for blending the images (between 0 and 1)\n";
        return 0;
    }
    if (argc != 6) {
        std::cerr << "Error: Incorrect number of arguments\n";
        std::cout << "Correct input: " << argv[0]
                  << " <algorithm version (base, cache, openmp, optimized)> <path to first image> <path to second image> <path to output> <weight> "
                  << std::endl;
        return 1;
    }

    std::string algorithm_version = argv[1];
    std::filesystem::path first_image = argv[2];
    std::filesystem::path second_image = argv[3];
    std::filesystem::path out_image = argv[4];
    float weight = std::stof(argv[5]);


    ImageMerger merger{};
    auto const start = std::chrono::high_resolution_clock::now();


    if (algorithm_version == "base") {
        std::cout << merger.merge_images(first_image, second_image, out_image, weight) << std::endl;
    } else if (algorithm_version == "openmp") {
        std::cout << merger.merge_images_openmp(first_image, second_image, out_image, weight) << std::endl;
    } else if (algorithm_version == "cache") {
        std::cout << merger.merge_images_cache(first_image, second_image, out_image, weight) << std::endl;
    } else if (algorithm_version == "optimized") {
        std::cout << merger.merge_images(first_image, second_image, out_image, weight) << std::endl;
    } else {
        std::cout << "Entered argument <" << algorithm_version
                  << "> is invalid. Supported algorithm versions are <base>, <openmp>, <cache> and <optimized>"
                  << std::endl;
    }


    auto const end = std::chrono::high_resolution_clock::now();
    auto const elapsed_time = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed_time.count() << std::endl;

}
