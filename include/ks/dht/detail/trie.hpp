// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace ks::dht::detail::inline abiv1 {

template<typename Key, typename T, std::size_t BucketSize>
class bitwise_trie final
{
public:
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<key_type, mapped_type>;

  class iterator;

public:
  bool push(value_type value, std::size_t max_depth)
  {
    auto * current = &root_node_;

    std::size_t depth{};
    for (auto const bit : value.first) {
      if (depth > max_depth)
        return false;

      if (auto leaf = as_leaf(current); leaf != nullptr) {

        if (leaf->size() < BucketSize) {
          leaf->push_back(std::move(value));
          return true;
        }

        auto splitted_leaf = split_leaf(std::move(*leaf), depth);

        children children{
          .zero{
              std::make_unique<node>(current, std::move(splitted_leaf.zero)) },
          .one{ std::make_unique<node>(current, std::move(splitted_leaf.one)) },
        };

        current->value = std::move(children);
      }

      if (value.first[bit])
        current = std::get<children>(current->value).one.get();
      else
        current = std::get<children>(current->value).zero.get();

      if (not is_close or subtree_depth != 0U)
        ++subtree_depth;
    }
  }

  bool remove(Key const& key)
  {
    return false;
  }

  iterator find(Key const& key)
  {
    return iterator{};
  }

  iterator end()
  {
    return iterator{};
  }

  bool empty() const noexcept
  {
    return true;
  }

private:
  using leaf = std::vector<value_type>;

  struct node;

  struct children final
  {
    std::unique_ptr<node> zero;
    std::unique_ptr<node> one;
  };

  struct node final
  {
    node * parent;
    std::variant<leaf, children> value;
  };

  struct split_result final
  {
    leaf one;
    leaf zero;
  };

private:
  leaf * as_leaf(node * node) {
    return std::get_if<leaf>(&node->value);
  }

  split_result split_leaf(leaf leaf, std::size_t depth)
  {
    split_result result;

    auto get_bit = [depth](auto const& value) { return value.first[depth]; };

    std::ranges::partition_copy(std::move(leaf),
                                std::back_inserter(result.one),
                                std::back_inserter(result.zero),
                                std::move(get_bit));

    return result;
  }

 private:
   node root_node_{ .parent{}, .content{} };
};

template<typename Key, typename T, std::size_t BucketSize>
class bitwise_trie<Key, T, BucketSize>::iterator final
{
public:
  using difference_type = std::ptrdiff_t;

  using value_type = typename bitwise_trie::value_type;

  using reference = value_type&;

  using pointer = value_type*;

  using iterator_category = std::forward_iterator_tag;

public:
  constexpr iterator()
  {
  }

  constexpr iterator(iterator const& o) = default;
  constexpr iterator& operator=(iterator const& o) = default;

  constexpr bool operator==(iterator const& o) const noexcept = default;

  constexpr iterator& operator++() noexcept
  {
    return *this;
  }

  constexpr iterator operator++(int) noexcept
  {
    auto old{ *this };

    operator++();

    return old;
  }

  reference operator*() const noexcept { static value_type v; return v; }

  pointer operator->() const noexcept { return std::addressof(operator*()); }

private:
};

} // namespace ks::dht::detail::inline abiv1
