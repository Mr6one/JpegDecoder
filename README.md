# JPEG Decoder Library
This repo contains an implemetation of JPEG decoder. Currently only baseline DCT and extended sequential DCT modes are supported. The project uses [fast and optimized DCT algorithm](https://github.com/Mr6one/FastDCT) as a thirdparty.

Decoding large 21600x10800px [Earth Topography](https://visibleearth.nasa.gov/images/73934/topography) image without chroma subsampling (YCbCr444) takes just 1.7 seconds on x64 Intel Core i7-9750H @ 2.60GHz CPU.

## Demo
To run demo follow the next steps:

```code
git clone --recurse-submodules https://github.com/Mr6one/JpegDecoder.git
cd JpegDecoder/demo
mkdir build && cd build
cmake .. && make -j4
./main path/to/jpeg/image
```

## Usage
To use library inside your project via CMake follow the next steps:

```cmake
add_subdirectory(/path/to/jpeg ${CMAKE_CURRENT_BINARY_DIR}/jpeg)

target_include_directories(${linking_target} PRIVATE ${jpeg_SOURCE_DIR}/include)
target_link_libraries(${linking_target} PRIVATE jpeg)
```
and then

```C++
#include "jpeg.hpp"
```

## References
1. Wallace G. K. The JPEG still picture compression standard //IEEE transactions on consumer electronics. – 1992. – Т. 38. – №. 1. – С. xviii-xxxiv.
2. Rabbani M., Joshi R. An overview of the JPEG 2000 still image compression standard //Signal processing: Image communication. – 2002. – Т. 17. – №. 1. – С. 3-48.
3. Hamilton E. JPEG file interchange format. – 2004.
