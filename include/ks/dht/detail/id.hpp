// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <cstdint>
#include <format>
#include <random>
#include <span>
#include <string>

#include <ks/serialization/serialize.hpp>

#include <ks/dht/detail/symbol_visibility.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

///
KS_DHT_EXPORT class id final
{
public:
  ///
  static constexpr std::size_t bit_size = 160;

  ///
  using block_type = std::uint8_t;

  /**
   *
   */
  template<typename BlockType>
  struct abstract_reference final
  {
    /**
     *
     */
    explicit operator bool const() const noexcept
    {
      return (current_block_ & mask_) != 0;
    }

    /**
     *
     */
    abstract_reference& operator=(bool value) noexcept
    {
      if (value)
        current_block_ |= mask_;
      else
        current_block_ &= ~mask_;

      return *this;
    }

    /**
     *
     */
    template<typename OtherBlockType>
    bool operator==(abstract_reference<OtherBlockType> const& o) const noexcept
    {
      return static_cast<bool>(o) == static_cast<bool>(*this);
    }

    ///
    BlockType& current_block_;
    ///
    block_type const mask_;
  };

  ///
  using reference = abstract_reference<block_type>;

  ///
  using const_reference = abstract_reference<block_type const>;

public:
  /**
   *  @brief Construct a null id.
   */
  constexpr id()
    : blocks_()
  {
  }

  /**
   *  @brief Construct a random id.
   */
  explicit id(std::default_random_engine& random_engine);

  /**
   *  @brief Construct an id from a string representation.
   */
  explicit id(std::string value);

  /**
   *  @brief Construct an id by hashing a value.
   */
  explicit id(std::span<std::byte const> value) noexcept;

  /**
   *
   */
  auto operator<=>(id const& o) const noexcept = default;

  /**
   *  @brief Return a const reference to a bit of the id.
   *  @param index The index of the bit (from 0 to bit_size - 1).
   *  @note Index 0 is the msb.
   */
  const_reference operator[](std::size_t index) const
  {
    return const_reference{ get_block(index), get_mask(index) };
  }

  /**
   *  @brief Return a reference to a bit of the id.
   *  @param index The index of the bit (from 0 to bit_size - 1).
   *  @note Index 0 is the msb.
   */
  reference operator[](std::size_t index)
  {
    return reference{ get_block(index), get_mask(index) };
  }

  id operator-(id const& other) const noexcept;

  bool operator<(id const& other) const noexcept;

private:
  ///
  static constexpr std::size_t byte_per_block = sizeof(block_type);

  ///
  static constexpr std::size_t bit_per_block = byte_per_block * 8;

  ///
  static constexpr std::size_t blocks_count = bit_size / bit_per_block;

  ///
  using blocks_type = std::array<block_type, blocks_count>;

  friend ks::serialization::serializer<id>;

  friend std::formatter<id>;

private:
  /**
   *
   */
  block_type const& get_block(std::size_t index) const noexcept
  {
    return blocks_[index / bit_per_block];
  }

  block_type& get_block(std::size_t index) noexcept
  {
    return blocks_[index / bit_per_block];
  }

  /**
   *
   */
  static block_type get_mask(std::size_t index) noexcept
  {
    return block_type{ 1 } << (bit_per_block - 1U - index % bit_per_block);
  }

private:
  ///
  blocks_type blocks_;
};

/**
 *
 */
inline id
distance(id const& a, id const& b)
{
  return a - b;
}

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
    // Skip leading 0.
    auto is_not_0 = [](auto b) { return b != 0; };
    auto i = std::ranges::find_if(id.blocks_, is_not_0);

    static constexpr std::size_t nibble_count = id::bit_per_block / 4U;

    for (auto e = id.blocks_.end(); i != e; ++i)
      ctx.advance_to(std::format_to(ctx.out(), "{:0{}x}", *i, nibble_count));

    return ctx.out();
  }
};
