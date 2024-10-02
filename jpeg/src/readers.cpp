#include "readers.hpp"

namespace jpeg {

/// MARK: BinaryReader

BinaryReader::BinaryReader(const std::string& path): file_(path, std::ios::binary), buffer_size_(page_size), byte_pos_(page_size) {}

bytes BinaryReader::read(size_t n_bytes) {
    bytes data;
    data.reserve(n_bytes);
    while (n_bytes > 0 && buffer_size_) {
        if (byte_pos_ >= buffer_size_) read_page();
        size_t read_size = std::min(buffer_size_ - byte_pos_, n_bytes);
        std::copy(buffer_ + byte_pos_, buffer_ + byte_pos_ + read_size, std::back_inserter(data));
        n_bytes -= read_size;
        byte_pos_ += read_size;
    }
    return data;
}

void BinaryReader::read_page() {
    file_.read(buffer_, page_size);
    buffer_size_ = file_.gcount();
    byte_pos_ = 0;
}

/// MARK: JpegReader

JpegReader::JpegReader(const std::string& path): BinaryReader{path} {
    bytes jpg_marker = read(2);
    if (jpg_marker[0] != DEFAULT_MARKER || jpg_marker[1] != JPG_MARKER) 
        throw std::runtime_error("wrong data format");
}

std::tuple<Marker, bytes> JpegReader::read_segment() {
    bytes marker = read(2);
    if (marker.size() < 2) throw std::runtime_error("reading error");
    if (marker[0] != DEFAULT_MARKER) throw std::runtime_error("wrong marker");
    if (marker[1] == END_MARKER) return {END_MARKER, {}};
    bytes size = read(2);
    if (size.size() < 2) throw std::runtime_error("reading error");
    size_t length = (size[0] << 8) | size[1];
    bytes data = read(length - 2);
    return {(Marker)marker[1], data};
}

/// MARK: BitStream

BitStream::BitStream(JpegReader& reader): reader_(reader), bit_pos_(0) {}

std::optional<bit> BitStream::read() {
    if (!bit_pos_) {
        byte_ = read_byte();
        if (byte_ == DEFAULT_MARKER) {
            auto marker = read_byte();
            if (marker == DNL_MARKER) {
                /// TODO: process DNL_MARKER
            } else if (marker != 0) {
                throw std::runtime_error("bitstream reading error");
            }
        }
        bit_pos_ = 8;
    }
    bit bit_ = (byte_ >> (--bit_pos_)) & 1;
    return bit_;
}

uint64_t BitStream::read(size_t n_bits) {
    uint64_t value = 0;
    for (size_t i = 0; i < n_bits; ++i) {
        value <<= 1;
        auto bit = read();
        if (!bit) throw std::runtime_error("bitstream reading error");
        value |= bit.value();
    }
    return value;
}

void BitStream::align_byte() {
    read(bit_pos_ % 8);
}

byte BitStream::read_byte() {
    auto bytes_ = reader_.read(1);
    if (bytes_.empty()) throw std::runtime_error("bitstream reading error");
    return bytes_[0];
}

}  // namespace jpeg
