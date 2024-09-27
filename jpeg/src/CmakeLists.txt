cmake_minimum_required(VERSION 3.29.0)

project(jpeg_src CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(SOURCES
    decoder.hpp
    decoder.cpp
    huffman.hpp
    huffman.cpp
    image.hpp
    image.cpp
    markers.hpp
    quantization.hpp
    quantization.cpp
    readers.hpp
    readers.cpp
    types.hpp
    utils.hpp
)

add_subdirectory(${jpeg_SOURCE_DIR}/3rdparty/dct/dct ${CMAKE_BINARY_DIR}/dct)

set(library jpeg)
add_library(${library})
target_sources(${library} PRIVATE ${SOURCES})
target_include_directories(${library} PRIVATE ${dct_SOURCE_DIR}/include)
target_include_directories(${library} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${library} PRIVATE dct)
