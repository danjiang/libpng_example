cmake_minimum_required(VERSION 3.15)
project(libpng_example)

set(CMAKE_CXX_STANDARD 11)

include_directories(libpng-1.6.37)
add_subdirectory(libpng-1.6.37)

include_directories(decoder/image)
add_executable(libpng_example main.cpp decoder/image/png_decoder.cpp)

target_link_libraries(libpng_example png_static)