#include "image.hpp"

#include <fstream>

namespace jpeg {

Image::Image(size_t height, size_t width, size_t channels): 
    height_(height), width_(width), channels_(channels), 
    data_(height * width * channels) {}

uint8_t* Image::data() {
    return data_.data();
}

std::array<size_t, 3> Image::size() const {
    return {height_, width_, channels_};
}

void Image::write_ppm(const std::string& path) const {
    std::ofstream out;
    out.open(path);
    auto format = channels_ == 1 ? "P5\n" : "P6\n";
    out << format << width_ << " " << height_ << "\n" << 255 << "\n";
    out.write((char*)data_.data(), width_ * height_ * channels_);
    out.close();
}

}  // namespace jpeg
