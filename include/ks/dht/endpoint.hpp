// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <variant>

#include <asio/ip/address_v4.hpp>
#include <asio/ip/address_v6.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename Ip>
struct basic_endpoint final
{
  Ip ip{};
  std::uint16_t port{ 27980u };
};

} // namespace detail

using endpoint_v4 = detail::basic_endpoint<asio::ip::address_v4>;
using endpoint_v6 = detail::basic_endpoint<asio::ip::address_v6>;
using endpoint = std::variant<endpoint_v4, endpoint_v6>;

} // namespace abiv1
} // namespace ks::dht
