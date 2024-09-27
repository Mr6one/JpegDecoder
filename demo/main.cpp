#include <iostream>

#include "jpeg.hpp"

int main(int argc, char* argv[]) {
    jpeg::Decoder decoder{argv[1]};
    auto image = decoder.decode();
    std::cout << "decoded: " << (image.has_value() ? "true" : "false") << std::endl;
    if (image) image->write_ppm("out.ppm");
    return 0;
}
