#include "decoder.hpp"

#include "utils.hpp"
#include "sdct.hpp"

#include <numeric>

namespace jpeg {

#define SCALED_DCT 0

constexpr static std::array<size_t, 64> zig_zag = {
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

DecoderImpl::DecoderImpl(const std::string& path): reader_(path), bitstream_(reader_) {
    prepare();
}

void DecoderImpl::prepare() {
    Marker marker; bytes data;
    while (std::tie(marker, data) = reader_.read_segment(), marker != END_MARKER) {
        switch (marker) {
            case DQT_MARKER:
                process_quantization_table(data);
                break;
            case DHT_MARKER:
                process_huffman_tree(data);
                break;
            case SOF0_MARKER:
            case SOF1_MARKER:
                process_frame(data);
                break;
            case SOF2_MARKER:
            case SOF3_MARKER:
            case SOF5_MARKER:
            case SOF6_MARKER:
            case SOF7_MARKER:
            case SOF9_MARKER:
            case SOF10_MARKER:
            case SOF11_MARKER: 
            case SOF13_MARKER: 
            case SOF14_MARKER: 
            case SOF15_MARKER:
                throw std::runtime_error("unsupported compression format");
            case DNL_MARKER:
                break;
            case DRI_MARKER:
                process_restart_interval(data);
                break;
            case DATA_MARKER:
                process_scan(data);
                return;
            default:
                break;
        }
    }
}

std::optional<Image> DecoderImpl::decode() {
    auto restart_cnt = meta_.restart_interval;
    auto total_mcu = meta_.n_mcu_x * meta_.n_mcu_y;
    for (size_t x = 0; x < meta_.n_mcu_x; ++x) {
        for (size_t y = 0; y < meta_.n_mcu_y; ++y) {
            for (size_t channel = 0; channel < meta_.channels; ++channel) {
                for (size_t cx = 0; cx < meta_.channels_meta[channel].nx_comp; ++cx) {
                    for (size_t cy = 0; cy < meta_.channels_meta[channel].ny_comp; ++cy) {
                        auto component = read_component(channel);
                        write_component(component, x, y, cx, cy, channel);
                    }
                }
            }
            --total_mcu;
            if (meta_.restart_interval && !(--restart_cnt)) {
                bitstream_.align_byte();
                std::fill(prev_dc_.begin(), prev_dc_.end(), 0);
                restart_cnt = meta_.restart_interval;
                if (total_mcu) {
                    auto marker = reader_.read_marker();
                    if ((marker & 0xF8) != RST0_MARKER) throw std::runtime_error("wrong marker");
                }
            }
        }
    }
    bitstream_.align_byte();
    auto marker = reader_.read_marker();
    if (marker == END_MARKER) {
        if (meta_.channels == 3) utils::YCbCrToRGB(image_);
        return image_;
    }
    return {};
}

std::array<float, 64> DecoderImpl::read_component(size_t channel) {
    auto quantization_table = quantization_tables_[meta_.channels_meta[channel].dqt_component].value();
    auto huffman_lookup_bitstream = [this](HuffmanTree& tree) {
        std::optional<size_t> code;
        while (!code) {
            auto bit_ = bitstream_.read();
            if (!bit_) throw std::runtime_error("unexpected EOF");
            code = tree.move(bit_.value());
        }
        return code.value();
    };
    auto extend_code = [](int value, size_t T) {
        int threshold = 1 << (T - 1);
        if (value < threshold) value += 1 - (1 << T);
        return value;
    };
    auto read_dc = [&, this](std::array<float, 64>& component) {
        auto length = huffman_lookup_bitstream(huffman_trees_[0][meta_.channels_meta[channel].dc_component].value());
        prev_dc_[channel] += extend_code(bitstream_.read(length), length);
        component[0] = quantization_table[0] * prev_dc_[channel];
#if SCALED_DCT
        component[0] *= sdct::scales[0] * sdct::scales[0];
#endif
    };
    auto read_ac = [&, this](std::array<float, 64>& component) {
        for (size_t K = 1; K < 64;) {
            auto code = huffman_lookup_bitstream(huffman_trees_[1][meta_.channels_meta[channel].ac_component].value());
            size_t zpad = (code >> 4) & 0xF;
            size_t length = code & 0xF;
            if (!length) {
                if (zpad == 0xF) {
                    K += 16;
                    continue;
                } else {
                    return;
                }
            }
            K += zpad;
            size_t i = zig_zag[K];
            component[i] = quantization_table[K++] * extend_code(bitstream_.read(length), length);
#if SCALED_DCT
            component[i] *= sdct::scales[i / 8] * sdct::scales[i % 8];
#endif
        }
    };
    std::array<float, 64> component;
    std::fill(component.begin(), component.end(), 0);
    read_dc(component);
    read_ac(component);
    idct(component);
    return component;
}

void DecoderImpl::write_component(const std::array<float, 64>& component, size_t x, size_t y, size_t cx, size_t cy, size_t channel) {
    auto mcu_height = meta_.mcu_height, mcu_width = meta_.mcu_width;
    auto component_height = meta_.channels_meta[channel].component_height;
    auto component_width = meta_.channels_meta[channel].component_width;
    auto horizontal_subsample = meta_.channels_meta[channel].horizontal_subsample;
    auto vertical_subsample = meta_.channels_meta[channel].vertical_subsample;

    auto mcu_col = cx * component_height + x * mcu_height;
    auto mcu_row = cy * component_width + y * mcu_width;
    auto subimage_size = meta_.width * meta_.channels;
    auto image_ptr = image_.data() + mcu_col * subimage_size + mcu_row * meta_.channels + channel;
    auto component_ptr = component.data();
    component_height = std::min(component_height, meta_.height - mcu_col);
    component_width = std::min(component_width, meta_.width - mcu_row);

    for (size_t i = 0; i < component_height; i += vertical_subsample) {
        for (size_t sx = 0; sx < vertical_subsample; ++sx) {
            auto row_image_ptr = image_ptr;
            auto row_component_ptr = component_ptr;
            for (size_t j = 0; j < component_width; j += horizontal_subsample) {
                for (size_t sy = 0; sy < horizontal_subsample; ++sy) {
                    *row_image_ptr = utils::clip8bit(*row_component_ptr + 128);
                    row_image_ptr += meta_.channels;
                }
                ++row_component_ptr;
            }
            image_ptr += subimage_size;
        }
        component_ptr += 8;
    }
}

void DecoderImpl::idct(std::array<float, 64>& component) {
    auto row_idct = [](std::array<float, 64>& component) {
        for (size_t i = 0; i < 8; ++i) sdct::idct<!SCALED_DCT>(component.data() + i * 8);
    };
    auto col_idct = [](std::array<float, 64>& component) {
        for (size_t i = 0; i < 8; ++i) sdct::idct<!SCALED_DCT>(component.data() + i, 8);
    };
    row_idct(component);
    col_idct(component);
}

void DecoderImpl::process_frame(const bytes& raw_data) {
    meta_.precision = raw_data[0];
    meta_.height = (raw_data[1] << 8) | raw_data[2];
    meta_.width = (raw_data[3] << 8) | raw_data[4];
    meta_.channels = raw_data[5];
    size_t mcu_height = 0, mcu_width = 0;
    meta_.channels_meta.resize(meta_.channels, ChannelMeta{});
    for (size_t c = 0; c < meta_.channels; ++c) {
        size_t shift = 3 * c + 6;
        size_t id = raw_data[shift] - 1;
        auto& channel_meta = meta_.channels_meta[id];
        channel_meta.ny_comp = (raw_data[shift + 1] >> 4) & 0xF;
        channel_meta.nx_comp = raw_data[shift + 1] & 0xF;
        channel_meta.dqt_component = raw_data[shift + 2];
        mcu_height = std::max(mcu_height, channel_meta.nx_comp);
        mcu_width = std::max(mcu_width, channel_meta.ny_comp);
    }
    mcu_height <<= 3; mcu_width <<= 3;
    for (size_t c = 0; c < meta_.channels; ++c) {
        auto& channel_meta = meta_.channels_meta[c];
        channel_meta.component_height = mcu_height / channel_meta.nx_comp;
        channel_meta.component_width = mcu_width / channel_meta.ny_comp;
        channel_meta.horizontal_subsample = channel_meta.component_height >> 3;
        channel_meta.vertical_subsample = channel_meta.component_width >> 3;
    }
    meta_.n_mcu_x = (meta_.height + mcu_height - 1) / mcu_height;
    meta_.n_mcu_y = (meta_.width + mcu_width - 1) / mcu_width;
    meta_.mcu_height = mcu_height;
    meta_.mcu_width = mcu_width;
    image_ = Image{meta_.height, meta_.width, meta_.channels};
    prev_dc_.resize(meta_.channels, 0);
}

void DecoderImpl::process_scan(const bytes& raw_data) {
    meta_.channels = raw_data[0];
    for (size_t c = 0; c < meta_.channels; ++c) {
        size_t shift = 2 * c + 1;
        size_t id = raw_data[shift] - 1;
        auto& channel_info = meta_.channels_meta[id];
        channel_info.dc_component = ((int)raw_data[shift + 1] >> 4) & 0xF;
        channel_info.ac_component = (int)raw_data[shift + 1] & 0xF;
    }
}

void DecoderImpl::process_huffman_tree(const bytes& raw_data) {
    size_t sot = 0;
    while (sot < raw_data.size()) {
        byte meta = raw_data[sot];
        size_t table_class = (meta >> 4) & 0xF;
        size_t table_id = meta & 0xF;
        bytes lengths = {raw_data.begin() + 1 + sot, raw_data.begin() + 17 + sot};
        size_t n_codes = std::accumulate(lengths.begin(), lengths.end(), 0);
        bytes codes = {raw_data.begin() + 17 + sot, raw_data.begin() + 17 + sot + n_codes};
        huffman_trees_[table_class][table_id].emplace(lengths, codes);
        sot += 17 + n_codes;
    }
}

void DecoderImpl::process_quantization_table(const bytes& raw_data) {
    size_t sot = 0;
    while (sot < raw_data.size()) {
        byte meta = raw_data[sot];
        size_t table_id = meta & 0xF;
        size_t byte_width = ((meta >> 4) & 0xF) + 1;
        quantization_tables_[table_id].emplace(bytes{raw_data.begin() + 1 + sot, raw_data.begin() + 1 + sot + byte_width * 64}, byte_width);
        sot += byte_width * 64 + 1;
    }
}

void DecoderImpl::process_restart_interval(const bytes& data) {
    meta_.restart_interval = (data[0] << 8) | data[1];
}

ImageMeta DecoderImpl::meta() {
    return meta_;
}

}  // namespace jpeg
