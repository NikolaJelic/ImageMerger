cmake_minimum_required(VERSION 3.25)
project(ImageMerger)

set(CMAKE_CXX_STANDARD 23)

add_executable(ImageMerger src/main.cpp src/image_merger.cpp src/image_merger.h src/bmp.cpp src/bmp.h)
