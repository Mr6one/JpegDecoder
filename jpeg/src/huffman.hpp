#pragma once

#include "types.hpp"

#include <optional>

namespace jpeg {

class HuffmanTree {

    struct Node {
        Node* left = nullptr;
        Node* right = nullptr;
        std::optional<int> code = std::nullopt;
    };

public:

    HuffmanTree(const std::vector<byte>& lengths, const std::vector<byte>& codes);

    std::optional<int> move(bit bit_);

private:

    bool add_node(Node* node, size_t height, int code);

private:
    Node* root_ = nullptr;
    Node* search_node_ = nullptr;
};

}  // namespace jpeg
