#include "quantization.hpp"

namespace jpeg {

QuantizationTable::QuantizationTable(const bytes& raw_data, size_t byte_width) {
    for (size_t i = 0, j = 0; i < raw_data.size(); i += byte_width, ++j) {
        if (byte_width == 2) {
            data_[j] = (raw_data[i] << 8) | raw_data[i + 1];
        } else { 
            data_[j] = raw_data[i]; 
        }
    }
}

uint16_t QuantizationTable::operator[](size_t i) const {
    return data_[i];
}

}  // namespace jpeg
