cmake_minimum_required(VERSION 3.20)
project(calico-c8)

set(CMAKE_CXX_STANDARD 17)

include_directories(${calico_c8_SOURCE_DIR}/src/)

file(GLOB SourceFiles "src/*.cc")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(calico-c8 ${SourceFiles})
target_link_libraries(calico-c8 ${SDL2_LIBRARIES})