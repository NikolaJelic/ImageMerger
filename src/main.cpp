#include <iostream>
#include "image_merger.h"
#include "bmp.h"
#include <functional>

int main(int argc, char *argv[]) {

    if (argc == 2 && std::string(argv[1]) == "help") {
        std::cout << "Correct input: " << argv[0]
                  << " <algorithm version (base, cache, opencl, optimized)> <merging method [average(default),max]> <path to first image> <path to second image> <path to output> <weight> "
                  << std::endl;
        std::cout << "Arguments:\n"
                  << "  algorithm version: the version of the algorithm to use (base, cache, opencl, optimized)\n"
                  << "  merging method: average will return the average pixel value with the added weight, max will take the value of the larger pixel\n"
                  << "  path to first image: the path to the first input image file\n"
                  << "  path to second image: the path to the second input image file\n"
                  << "  path to output: the path to the output image file\n"
                  << "  weight: the weight to use for blending the images (between 0 and 1)\n";
        return 0;
    }
    if (argc != 7) {

        std::cerr << "Error: Incorrect number of arguments\n";
        std::cout << "Correct input: " << argv[0]
                  << " <algorithm version (base, cache, openmp, optimized)> <merging method [average(default),max]> <path to first image> <path to second image> <path to output> <weight> "
                  << std::endl;
        return 1;
    }

    std::string algorithm_version = argv[1];
    int i = 0;
    std::string merge_method{"average"};
    if (std::filesystem::path{argv[2]}.extension() != ".bmp") {
        merge_method = argv[2];
        ++i;
    }
    std::filesystem::path first_image = argv[2 + i];
    std::filesystem::path second_image = argv[3 + i];
    std::filesystem::path out_image = argv[4 + i];
    float weight = std::stof(argv[5 + i]);
    if (weight > 1.0 || weight < 0.0) {
        std::cerr << "Weight value must be in range [0.0,1.0].\n";
        return 1;
    }


    ImageMerger merger{};

    int merge_val = (merge_method == "max") ? 0 : 1;

    auto start = std::chrono::high_resolution_clock::now();
    merger.merge_images(merge_val, first_image, second_image, "../resources/out/test0.bmp", weight);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_time = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed_time.count() << std::endl;
    std::cout << "=======================================\n Cached:\n";
    start = std::chrono::high_resolution_clock::now();
    merger.merge_images_cache(merge_val, first_image, second_image, "../resources/out/test1.bmp", weight);
    end = std::chrono::high_resolution_clock::now();
    elapsed_time = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed_time.count() << std::endl;
    std::cout << "=======================================\n openmp:\n";
    start = std::chrono::high_resolution_clock::now();
    merger.merge_images_openmp(merge_val, first_image, second_image, "../resources/out/test2.bmp", weight);
    end = std::chrono::high_resolution_clock::now();
    elapsed_time = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed_time.count() << std::endl;
    std::cout << "=======================================\n Optimized\n";
    start = std::chrono::high_resolution_clock::now();
    merger.merge_images_optimized(1, first_image, second_image, "../resources/out/test1.bmp", weight);
    end = std::chrono::high_resolution_clock::now();
    elapsed_time = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed_time.count() << std::endl;
    std::cout << "=======================================\n \n";



    /*   if (algorithm_version == "base") {
           std::cout << merger.merge_images(merge_val, first_image, second_image, out_image, weight) << std::endl;
       } else if (algorithm_version == "openmp") {
           std::cout << merger.merge_images_openmp(merge_val, first_image, second_image, out_image, weight)
                     << std::endl;
       } else if (algorithm_version == "cache") {
           std::cout << merger.merge_images_cache(merge_val, first_image, second_image, out_image, weight)
                     << std::endl;
       } else {
           std::cout << "Entered argument <" << algorithm_version
                     << "> is invalid. Supported algorithm versions are <base>, <openmp>, <cache> and <optimized>"
                     << std::endl;
       }*/




}
