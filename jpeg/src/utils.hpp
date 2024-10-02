#pragma once

#include "image.hpp"

#include <cmath>
#include <tuple>
#include <array>

namespace jpeg {

namespace utils {

template<typename T>
uint8_t clip8bit(const T& value) {
    return std::clamp<T>(value, 0, 0xFF);
}

void YCbCrToRGB(Image& image) {
    auto pixel_conversion = [](float Y, float Cb, float Cr) -> std::tuple<uint8_t, uint8_t, uint8_t> {
        Cb -= 128;
        Cr -= 128;
        uint8_t R = clip8bit(std::round(Y + 1.402 * Cr));
        uint8_t G = clip8bit(std::round(Y - 0.34414 * Cb - 0.71414 * Cr));
        uint8_t B = clip8bit(std::round(Y + 1.772 * Cb));
        return {R, G, B};
    };
    auto image_ptr = image.data();
    auto [height, width, _] = image.size();
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            std::tie(
                *(image_ptr),
                *(image_ptr + 1),
                *(image_ptr + 2)
            ) = pixel_conversion(
                *(image_ptr) + 0.0f,
                *(image_ptr + 1) + 0.0f,
                *(image_ptr + 2) + 0.0f
            );
            image_ptr += 3;
        }
    }
}

}  // namespace utils

}  // namespace jpeg
