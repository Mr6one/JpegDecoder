#pragma once

#include <array>
#include <vector>
#include <optional>

#include "types.hpp"
#include "image.hpp"
#include "readers.hpp"
#include "huffman.hpp"
#include "quantization.hpp"

namespace jpeg {

struct ChannelMeta {
    size_t horizontal_subsample = 0;
    size_t vertical_subsample = 0;
    size_t nx_comp = 0;
    size_t ny_comp = 0;
    size_t component_height = 0;
    size_t component_width = 0;
    size_t dqt_component = 0;
    size_t dc_component = 0;
    size_t ac_component = 0;
};

struct ImageMeta {
    std::vector<ChannelMeta> channels_meta;
    size_t height = 0;
    size_t width = 0;
    size_t channels = 0;
    size_t precision = 0;
    size_t restart_interval = 0;
    size_t n_mcu_x = 0;
    size_t n_mcu_y = 0;
    size_t mcu_height = 0;
    size_t mcu_width = 0;
};

class DecoderImpl {
    using HuffmanTrees = std::array<std::array<std::optional<HuffmanTree>, 4>, 2>;
    using QuantizationTables = std::array<std::optional<QuantizationTable>, 4>;
public:

    DecoderImpl(const std::string& path);

    std::optional<Image> decode();

    ImageMeta meta();

private:

    void prepare();

    void write_component(const std::array<float, 64>& component, size_t x, size_t y, size_t cx, size_t cy, size_t channel);

    std::array<float, 64> read_component(size_t channel);

    void idct(std::array<float, 64>& component);

    void process_frame(const bytes& raw_data);

    void process_scan(const bytes& raw_data);

    void process_huffman_tree(const bytes& raw_data);

    void process_quantization_table(const bytes& raw_data);

    void process_restart_interval(const bytes& data);

private:
    JpegReader reader_;
    BitStream bitstream_;
    HuffmanTrees huffman_trees_;
    QuantizationTables quantization_tables_;
    ImageMeta meta_;
    Image image_;
    std::vector<int> prev_dc_;
};

}  // namespace jpeg
