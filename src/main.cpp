#include <iostream>
#include "image_merger.h"
#include "bmp.h"

int main() {
    ImageMerger merger{};

    Bmp bmp{"/home/nikola/projects/cpp/ImageMerger/resources/zelena_zgrada.bmp"};
    std::cout << sizeof(bmp.getHeader()) << std::endl;
    merger.merge_images("/home/nikola/projects/cpp/ImageMerger/resources/zelena_zgrada.bmp",
                        "/home/nikola/projects/cpp/ImageMerger/resources/blackbuck.bmp", "example4.bmp", 0.9f);

}
