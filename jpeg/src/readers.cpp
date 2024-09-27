#include "readers.hpp"

namespace jpeg {

/// MARK: BinaryReader

BinaryReader::BinaryReader(const std::string& path): file_(path, std::ios::binary) {}

bytes BinaryReader::read(size_t n_bytes) {
    bytes data;
    data.reserve(n_bytes);
    while (n_bytes > 0 && !file_.eof()) {
        auto buffer = read_subpage(std::min(n_bytes, page_size));
        n_bytes -= buffer.size();
        data.insert(data.end(), std::make_move_iterator(buffer.begin()), std::make_move_iterator(buffer.end()));
    }
    return data;
}

bytes BinaryReader::read_page() {
    return read_subpage(page_size);
}

bytes BinaryReader::read_subpage(size_t n_bytes) {
    if (file_.eof()) return {};
    file_.read(buffer_, n_bytes);
    size_t bytes_read = file_.gcount();
    return {buffer_, buffer_ + bytes_read};
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

BitStream::BitStream(JpegReader& reader): bit_pos_(0), byte_pos_(0), reader_(reader) {}

std::optional<bit> BitStream::read() {
    if (!bit_pos_) {
        auto raw_byte_ = read_byte();
        if (!raw_byte_) return {};
        byte_ = raw_byte_.value();
        if (byte_ == DEFAULT_MARKER) {
            auto raw_byte_ = read_byte();
            if (raw_byte_ && raw_byte_.value() != 0) {
                byte_ <<= 8;
                byte_ |= raw_byte_.value();
                bit_pos_ += 8;
            }
        }
        bit_pos_ += 8;
    }
    bit bit_ = (byte_ >> (--bit_pos_)) & 1;
    return bit_;
}

uint64_t BitStream::read(size_t n_bits) {
    uint64_t value = 0;
    for (size_t i = 0; i < n_bits; ++i) {
        value <<= 1;
        value |= read().value();
    }
    return value;
}

void BitStream::align_byte() {
    read(bit_pos_ % 8);
}

std::optional<byte> BitStream::read_byte() {
    if (byte_pos_ >= data_.size()) {
        data_ = reader_.read_page();
        byte_pos_ = 0;
        if (data_.empty()) return {};
    }
    return data_[byte_pos_++];
}

}  // namespace jpeg
