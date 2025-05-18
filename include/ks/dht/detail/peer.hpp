// SPDX-License-Identifier: MIT

#pragma once

#include <format>
#include <system_error>

#include <ks/serialization/serialize.hpp>

#include <ks/dht/detail/id.hpp>
#include <ks/dht/endpoint.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

using peer_id = id;
using peer_endpoint = endpoint;

struct peer final
{
  peer_id id;
  peer_endpoint endpoint;

  bool operator==(peer const&) const noexcept = default;
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht

template<>
struct std::formatter<ks::dht::detail::peer> final
{
  constexpr auto parse(auto& ctx) const { return ctx.begin(); }

  auto format(ks::dht::detail::peer const& peer, auto& ctx) const
  {
    return std::format_to(ctx.out(), "{}/{}", peer.id, peer.endpoint);
  }
};

template<>
struct ks::serialization::serializer<ks::dht::detail::peer> final
{
  std::error_code operator()(auto& archive, auto& peer) const
  {
    return serialize(archive, peer.id, peer.endpoint);
  }
};
