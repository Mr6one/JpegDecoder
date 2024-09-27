cmake_minimum_required(VERSION 3.29.0)

project(demo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

add_subdirectory(${demo_SOURCE_DIR}/../jpeg ${CMAKE_CURRENT_BINARY_DIR}/jpeg)

set(executable main)
add_executable(${executable} main.cpp)
target_include_directories(${executable} PRIVATE ${jpeg_SOURCE_DIR}/include)
target_link_libraries(${executable} PRIVATE jpeg)
