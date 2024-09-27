#pragma once

#include <optional>

#include "decoder.hpp"

namespace jpeg {

class Decoder {
public:

    Decoder(const std::string& path): decoder_(path) {}
    Decoder(const Decoder&) = delete;
    Decoder(Decoder&&) = default;
    Decoder& operator=(const Decoder&) = delete;
    Decoder& operator=(Decoder&&) = delete;
    ~Decoder() = default;

    std::optional<Image> decode() {
        return decoder_.decode();
    }

    ImageMeta meta() {
        return decoder_.meta();
    }

private:
    DecoderImpl decoder_;
};

}  // namespace jpeg
