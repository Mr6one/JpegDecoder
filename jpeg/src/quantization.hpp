#pragma once

#include <array>

#include "types.hpp"

namespace jpeg {

class QuantizationTable {
public:

    QuantizationTable(const bytes& raw_data, size_t byte_width);

    uint16_t operator[](size_t i) const;

private:
    std::array<uint16_t, 64> data_;
};

}  // namespace jpeg
