#include "huffman.hpp"

namespace jpeg {

HuffmanTree::HuffmanTree(const std::vector<byte>& lengths, const std::vector<byte>& codes): root_{new Node()}, search_node_(root_) {
    size_t j = 0;
    for (size_t i = 0; i < lengths.size(); ++i) {
        size_t count = lengths[i];
        while (count > 0) {
            add_node(root_, i + 1, codes[j++]);
            --count;
        }
    }
}

std::optional<int> HuffmanTree::move(bit bit_) {
    if (bit_) search_node_ = search_node_->right;
    else search_node_ = search_node_->left;
    if (!search_node_) throw std::runtime_error("huffman error");
    auto code = search_node_->code;
    if (code) search_node_ = root_;
    return code;
}

bool HuffmanTree::add_node(Node* node, size_t height, int code) {
    if (node->code) return false;
    if (height == 0) {
        node->code = code;
        return true;
    }
    if (!node->left) node->left = new Node();
    bool status = add_node(node->left, height - 1, code);
    if (!status) {
        if (!node->right) node->right = new Node();
        return add_node(node->right, height - 1, code);
    }
    return status;
}

}  // namespace jpeg
