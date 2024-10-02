#pragma once

#include "types.hpp"
#include "readers.hpp"

#include <optional>

namespace jpeg {

class HuffmanTree {

    struct Node {
        Node* left = nullptr;
        Node* right = nullptr;
        std::optional<int> code = std::nullopt;
    };

public:

    HuffmanTree(const bytes& lengths, const bytes& codes);

    int lookup(BitStream& reader);

private:

    std::optional<int> add_node(Node* node, size_t height, int code = 0);

private:
    Node* root_ = nullptr;
    bytes values_;
    std::vector<int> codes_;
    std::vector<int> min_vals_;
    std::vector<int> max_vals_;
    std::vector<size_t> valptr_;
};

}  // namespace jpeg
