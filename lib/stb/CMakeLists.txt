cmake_minimum_required(VERSION 3.12)
project(stb)

set(CMAKE_CXX_STANDARD 17)

add_library(stb src/stb_image.cpp)

target_include_directories(stb PUBLIC .)