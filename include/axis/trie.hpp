#include <array>
#include <cstddef>
#include <memory>
#include <string_view>

struct Trie {
  struct Node {
    bool terminal = false;
    std::array<std::unique_ptr<Node>, 256> next{};
  };

  Node root;

  void insert(std::string_view s) {
    Node *cur = &root;
    for (unsigned char c : s) {
      auto &child = cur->next[c];
      if (!child)
        child = std::make_unique<Node>();
      cur = child.get();
    }
    cur->terminal = true;
  }

  bool contains(std::string_view s) const {
    auto *cur = &root;
    for (unsigned char c : s) {
      const auto &child = cur->next[c];
      if (!child)
        return false;
      cur = child.get();
    }
    return cur->terminal;
  }
};
