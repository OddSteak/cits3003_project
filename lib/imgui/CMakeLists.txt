cmake_minimum_required(VERSION 3.12)
project(imgui)

set(CMAKE_CXX_STANDARD 17)

add_library(imgui
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_demo.cpp
)

target_include_directories(imgui PUBLIC . imgui)