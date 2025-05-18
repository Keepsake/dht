// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <functional>

#include <openssl/sha.h>

#include <ks/dht/detail/id.hpp>
#include <ks/dht/error.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

namespace {

id::block_type
hex_to_block(std::string_view value)
{
  id::block_type result;
  auto begin = value.data(), end = begin + value.size();
  auto [ptr, failure] = std::from_chars(begin, end, result, 16);

  if (ptr != end or failure != std::errc{}) [[unlikely]]
    throw std::system_error{ error::invalid_id };

  return result;
}

} // namespace

id::id() noexcept = default;

id::id(std::default_random_engine& random_engine)
{
  using limits = std::numeric_limits<block_type>;
  std::uniform_int_distribution<block_type> distribution(limits::min(),
                                                         limits::max());

  std::ranges::generate(blocks_, [&] { return distribution(random_engine); });
}

id::id(std::string_view value)
{
  constexpr std::size_t hex_char_per_block = byte_per_block * 2U;

  auto out = blocks_.rbegin();

  while (not value.empty()) {
    if (out == blocks_.rend()) [[unlikely]]
      throw std::system_error{ error::invalid_id };

    auto const count = std::min(value.size(), hex_char_per_block);
    *out++ = hex_to_block(value.substr(value.size() - count));
    value.remove_suffix(count);
  }

  std::ranges::fill(out, blocks_.rend(), 0U);
}

id::id(std::span<std::byte const> value) noexcept
{
  // Use OpenSSL crypto hash.
  SHA256(reinterpret_cast<std::uint8_t const*>(value.data()),
         value.size(),
         reinterpret_cast<std::uint8_t*>(blocks_.data()));
}

bool
id::operator[](std::size_t index) const noexcept
{
  assert(index < bit_size);

  constexpr auto msb = block_type{ 1U } << (bit_per_block - 1U);

  auto const bit_index = index % bit_per_block;
  auto const mask = msb >> bit_index;

  auto const block_index = index / bit_per_block;
  assert(block_index < blocks_.size());

  return blocks_[block_index] & mask;
}

id
id::operator^(id const& other) const noexcept
{
  id result;

  std::ranges::transform(
      blocks_, other.blocks_, result.blocks_.begin(), std::bit_xor<>{});

  return result;
}

bool
id::operator<(id const& other) const noexcept
{
  return std::ranges::lexicographical_compare(blocks_, other.blocks_);
}

id::const_iterator
id::begin() const noexcept
{
  return { this, 0 };
}

id::const_iterator
id::end() const noexcept
{
  return { this, id::bit_size };
}

std::reverse_iterator<id::const_iterator>
id::rbegin() const noexcept
{
  return std::make_reverse_iterator(end());
}

std::reverse_iterator<id::const_iterator>
id::rend() const noexcept
{
  return std::make_reverse_iterator(begin());
}

std::size_t
id::countl_zero() const noexcept
{
  std::size_t total{};

  for (block_type const block : blocks_) {
    std::size_t const count = std::countl_zero(block);
    total += count;
    if (count != bit_per_block)
      break;
  }

  return total;
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
