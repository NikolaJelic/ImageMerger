cmake_minimum_required(VERSION 3.25)
project(ImageMerger)

set(CMAKE_CXX_STANDARD 23)

add_executable(ImageMerger src/main.cpp src/image_merger.cpp src/image_merger.h src/bmp.cpp src/bmp.h)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Ofast")
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ImageMerger PUBLIC OpenMP::OpenMP_CXX)
endif()
