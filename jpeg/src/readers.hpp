#pragma once

#include <fstream>
#include <optional>

#include "types.hpp"
#include "markers.hpp"

namespace jpeg {

class BinaryReader {
    constexpr static size_t page_size = 4096;
public:

    BinaryReader(const std::string& path);

    bytes read(size_t n_bytes);

    bytes read_page();

private:

    bytes read_subpage(size_t n_bytes);

private:
    std::ifstream file_;
    char buffer_[page_size];
};

class JpegReader: public BinaryReader {
public:

    JpegReader(const std::string& path);

    std::tuple<Marker, bytes> read_segment();
};

class BitStream {
public:

    BitStream(JpegReader& reader);

    std::optional<bit> read();

    uint64_t read(size_t n_bits);

    void align_byte();

private:

    std::optional<byte> read_byte();

private:
    size_t bit_pos_;
    size_t byte_pos_;
    JpegReader& reader_;
    bytes data_;
    uint64_t byte_;
};

}  // namespace jpeg
