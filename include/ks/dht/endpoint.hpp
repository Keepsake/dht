// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <format>
#include <utility>
#include <variant>

#include <asio/ip/address_v4.hpp>
#include <asio/ip/address_v6.hpp>

#include <ks/serialization/serialize.hpp>
#include <ks/serialization/std.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename Ip>
struct basic_endpoint final
{
  Ip ip{};
  std::uint16_t port{};

  bool operator==(basic_endpoint const&) const noexcept = default;
};

} // namespace detail

using endpoint_v4 = detail::basic_endpoint<asio::ip::address_v4>;
using endpoint_v6 = detail::basic_endpoint<asio::ip::address_v6>;
using endpoint = std::variant<endpoint_v4, endpoint_v6>;

} // namespace abiv1
} // namespace ks::dht

template<>
struct ks::serialization::serializer<asio::ip::address_v4> final
{
  std::error_code operator()(ks::serialization::oarchive& archive,
                             auto const& address) const
  {
    asio::ip::address_v4::bytes_type const bytes = address.to_bytes();
    return serialize(archive, bytes);
  }

  std::error_code operator()(ks::serialization::iarchive& archive,
                             auto& address) const
  {
    asio::ip::address_v4::bytes_type bytes;
    if (auto const failure = serialize(archive, bytes); failure) [[unlikely]]
      return failure;

    address = asio::ip::make_address_v4(bytes);

    return {};
  }
};

template<>
struct ks::serialization::serializer<asio::ip::address_v6> final
{
  std::error_code operator()(ks::serialization::oarchive& archive,
                             auto const& address) const
  {
    asio::ip::address_v6::bytes_type const bytes = address.to_bytes();
    return serialize(archive, bytes);
  }

  std::error_code operator()(ks::serialization::iarchive& archive,
                             auto& address) const
  {
    asio::ip::address_v6::bytes_type bytes;
    if (auto const failure = serialize(archive, bytes); failure) [[unlikely]]
      return failure;

    address = asio::ip::make_address_v6(bytes);

    return {};
  }
};

template<typename Ip>
struct ks::serialization::serializer<ks::dht::detail::basic_endpoint<Ip>> final
{
  std::error_code operator()(auto& archive, auto& endpoint) const
  {
    return serialize(archive, endpoint.ip, endpoint.port);
  }
};

template<typename Ip>
struct std::formatter<ks::dht::detail::basic_endpoint<Ip>> final
{
  constexpr auto parse(auto& ctx) const { return ctx.begin(); }

  auto format(auto const& endpoint, auto& ctx) const
  {
    return std::format_to(
        ctx.out(), "{}:{}", endpoint.ip.to_string(), endpoint.port);
  }
};

template<>
struct std::formatter<ks::dht::endpoint> final
{
  constexpr auto parse(auto& ctx) const { return ctx.begin(); }

  auto format(auto const& endpoint, auto& ctx) const
  {
    auto print = [&ctx](auto const& e) {
      return std::format_to(ctx.out(), "{}:{}", e.ip.to_string(), e.port);
    };

    return std::visit(std::move(print), endpoint);
  }
};
