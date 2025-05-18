// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <format>
#include <functional>
#include <iterator>
#include <random>
#include <ranges>
#include <span>
#include <string_view>

#include <ks/serialization/serialize.hpp>

#include <ks/dht/detail/symbol_visibility.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

KS_DHT_EXPORT class id final
{
public:
  static constexpr std::size_t bit_size = 256U;

  using block_type = std::uint64_t;

  class const_iterator;

public:
  id() noexcept;

  explicit id(std::default_random_engine& random_engine);

  explicit id(std::string_view value);

  explicit id(std::span<std::byte const> value) noexcept;

  auto operator<=>(id const& o) const noexcept = default;

  // index 0 is MSB
  bool operator[](std::size_t index) const noexcept;

  id operator^(id const& other) const noexcept;

  bool operator<(id const& other) const noexcept;

  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;

  std::reverse_iterator<const_iterator> rbegin() const noexcept;
  std::reverse_iterator<const_iterator> rend() const noexcept;

  std::size_t countl_zero() const noexcept;

private:
  static constexpr std::size_t byte_per_block = sizeof(block_type);
  static constexpr std::size_t bit_per_block = byte_per_block * 8U;
  static constexpr std::size_t blocks_count = bit_size / bit_per_block;

  using blocks_type = std::array<block_type, blocks_count>;

  friend ks::serialization::serializer<id>;
  friend std::formatter<id>;
  friend std::hash<id>;

private:
  blocks_type blocks_{};
};

class id::const_iterator final
{
public:
  using difference_type = std::ptrdiff_t;
  using value_type = bool;
  using reference = bool;
  using iterator_category = std::random_access_iterator_tag;

public:
  constexpr const_iterator() noexcept = default;

  constexpr const_iterator(id const* id, difference_type index) noexcept
    : id_{ id }
    , index_{ index }
  {
  }

  auto operator<=>(const_iterator const& o) const noexcept = default;

  constexpr value_type operator*() const noexcept { return (*id_)[index_]; }

  constexpr const_iterator& operator+=(difference_type diff) noexcept
  {
    index_ += diff;
    return *this;
  }

  constexpr const_iterator operator+(difference_type diff) noexcept
  {
    return { id_, index_ + diff };
  }

  constexpr const_iterator& operator++() noexcept
  {
    ++index_;
    return *this;
  }

  constexpr const_iterator operator++(int) noexcept
  {
    auto old{ *this };
    ++index_;
    return old;
  }

  constexpr const_iterator& operator-=(difference_type diff) noexcept
  {
    index_ -= diff;
    return *this;
  }

  constexpr const_iterator operator-(difference_type diff) noexcept
  {
    return { id_, index_ - diff };
  }

  constexpr const_iterator& operator--() noexcept
  {
    --index_;
    return *this;
  }

  constexpr const_iterator operator--(int) noexcept
  {
    auto old{ *this };
    --index_;
    return old;
  }

  constexpr difference_type operator-(const_iterator const& o) const noexcept
  {
    return index_ - o.index_;
  }

  constexpr reference operator[](difference_type diff) const noexcept
  {
    return (*id_)[index_ + diff];
  }

  const_iterator& operator=(const_iterator const&) noexcept = default;

private:
  id const* id_;
  difference_type index_;
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht

template<>
struct ks::serialization::serializer<ks::dht::detail::id> final
{
  std::error_code operator()(auto& archive, auto& id) const
  {
    return serialize(archive, id.blocks_);
  }
};

template<>
struct std::formatter<ks::dht::detail::id> final
{
  using id = ks::dht::detail::id;

  constexpr auto parse(auto& ctx) const { return ctx.begin(); }

  auto format(id const& id, auto& ctx) const
  {
    auto is_not_0 = [](auto b) { return b != 0; };
    auto i = std::ranges::find_if(id.blocks_, is_not_0);

    constexpr std::size_t hex_char_per_block = id::byte_per_block * 2U;

    for (auto e = id.blocks_.end(); i != e; ++i)
      ctx.advance_to(
          std::format_to(ctx.out(), "{:0{}x}", *i, hex_char_per_block));

    return ctx.out();
  }
};

template<>
struct std::hash<ks::dht::detail::id> final
{
  using value_type = ks::dht::detail::id;

  std::size_t operator()(value_type const& id) const
  {
    std::hash<value_type::block_type> hasher{};

    auto hash_combine = [](std::size_t state, std::size_t hash) {
      constexpr auto golden_ratio = 0x9e3779b9U;
      return state ^ hash + golden_ratio + (state << 6) + (state >> 2);
    };

    std::size_t initial_state{};
    return std::ranges::fold_left(id.blocks_ | std::views::transform(hasher),
                                  initial_state,
                                  hash_combine);
  }
};
