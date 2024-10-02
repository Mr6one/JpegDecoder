#include "huffman.hpp"

#include <stdexcept>

namespace jpeg {

HuffmanTree::HuffmanTree(const bytes& lengths, const bytes& values): 
    root_{new Node()}, values_(values), codes_(values.size()), 
        min_vals_(lengths.size(), -1), max_vals_(lengths.size(), -1), 
        valptr_(lengths.size(), 0) {
    size_t j = 0;
    for (size_t i = 0; i < lengths.size(); ++i) {
        size_t count = lengths[i];
        while (count > 0) {
            codes_[j++] = add_node(root_, i + 1).value();
            --count;
        }
        count = lengths[i];
        if (count) {
            min_vals_[i] = codes_[j - count];
            max_vals_[i] = codes_[j - 1];
            valptr_[i] = j - count;
        }
    }
}

int HuffmanTree::lookup(BitStream& reader) {
    int value = 0;
    for (size_t i = 0; i < max_vals_.size(); ++i) {
        value = (value << 1) | reader.read(1);
        if (value <= max_vals_[i]) return values_[valptr_[i] + value - min_vals_[i]];
    }
    throw std::runtime_error("huffman decoding error");
    return -1;
}

std::optional<int> HuffmanTree::add_node(Node* node, size_t height, int code) {
    if (node->code) return {};
    if (height == 0) {
        node->code = code;
        return code;
    }
    if (!node->left) node->left = new Node();
    auto status = add_node(node->left, height - 1, code << 1);
    if (!status) {
        if (!node->right) node->right = new Node();
        return add_node(node->right, height - 1, (code << 1) | 1);
    }
    return status;
}

}  // namespace jpeg
