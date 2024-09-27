#pragma once

#include <vector>

namespace jpeg {

class Slice {
public:
    Slice() = default;
private:
    
};

class Image {
public:

    Image() = default;

    Image(size_t height, size_t width, size_t channels);

    std::array<size_t, 3> size() const;

    uint8_t* data();

    void write_ppm(const std::string& path) const;

private:
    size_t height_;
    size_t width_;
    size_t channels_;
    std::vector<uint8_t> data_;
};

}  // namespace jpeg
