// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>

#include <ks/dht/detail/id.hpp>
#include <ks/dht/detail/peer.hpp>
#include <ks/dht/endpoint.hpp>

inline ks::dht::endpoint
create_endpoint(std::string const& ip = std::string{ "127.0.0.1" },
                std::uint16_t const& service = 12345)
{
  return ks::dht::endpoint_v4{
    .ip = asio::ip::address_v4::from_string(ip),
    .port = service
  };
}

inline ks::dht::detail::peer
create_peer(ks::dht::detail::id const& id = ks::dht::detail::id{},
            ks::dht::endpoint const& endpoint = create_endpoint())
{
  return { id, endpoint };
}
