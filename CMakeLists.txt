cmake_minimum_required(VERSION 3.20)
project(RayTracing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(raytracing src/main.cpp
    src/vec3.h
    src/color.h
    src/ray.h)

include_directories(src)
