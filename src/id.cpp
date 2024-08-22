// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <utility>

#include <openssl/sha.h>

#include <ks/dht/detail/id.hpp>
#include <ks/dht/error.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

namespace {

id::block_type
to_block(std::string const& s)
{
  auto is_id_digit = [](int c) { return std::isxdigit(c); };
  if (!std::all_of(s.begin(), s.end(), is_id_digit))
    throw std::system_error{ error::invalid_id };

  std::stringstream converter{ s };

  std::uint64_t result;
  converter >> std::hex >> result;

  assert(!converter.fail() && "hexa to decimal conversion failed");

  return static_cast<id::block_type>(result);
}

} // namespace

id::id(std::default_random_engine& random_engine)
{
  // The output of the generator is treated as boolean value.
  std::uniform_int_distribution<> distribution(
      std::numeric_limits<block_type>::min(),
      std::numeric_limits<block_type>::max());

  std::generate(blocks_.begin(), blocks_.end(), [&] {
    return distribution(random_engine);
  });
}

id::id(std::string s)
{
  static constexpr std::size_t hex_char_per_block = id::byte_per_block * 2;
  static constexpr std::size_t string_max_size =
      blocks_count * hex_char_per_block;

  if (s.size() > string_max_size)
    throw std::system_error{ error::invalid_id };

  // Insert leading 0.
  s.insert(s.begin(), string_max_size - s.size(), '0');

  assert(s.size() == string_max_size && "string padding failed");
  for (std::size_t i = 0; i != blocks_count; ++i)
    blocks_[i] = to_block(s.substr(i * hex_char_per_block, hex_char_per_block));
}

id::id(std::span<std::byte const> value) noexcept
{
  // Use OpenSSL crypto hash.
  SHA1(reinterpret_cast<std::uint8_t const*>(value.data()),
       value.size(),
       blocks_.data());
}

id
id::operator-(id const& other) const noexcept
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

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
