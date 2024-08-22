// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <asio/buffer.hpp>
#include <asio/execution/executor.hpp>
#include <asio/ip/v6_only.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

#ifdef _MSC_VER
template<typename Completion>
constexpr void
async_receive_from(asio::ip::udp::socket& socket,
                   asio::ip::udp::endpoint const& endpoint,
                   auto buffer,
                   Completion&& completion)
{
  auto on_read = [&socket,
                  &endpoint,
                  buffer,
                  completion = std::forward<Completion>(completion)](
                     std::error_code failure,
                     std::size_t bytes_received) mutable {
    // On Windows, an UDP socket may return connection_reset
    // to inform application that a previous send by this socket
    // has generated an ICMP port unreachable.
    // https://msdn.microsoft.com/en-us/library/ms740120.aspx
    // Ignore it and schedule another read.
    if (failure == std::errc::connection_reset) {
      async_receive_from(socket, endpoint, buffer, std::move(completion));
      return;
    }

    std::move(completion)(failure, bytes_received);
  };

  socket.async_receive_from(buffer, endpoint, std::move(on_read));
}

template<typename Completion>
constexpr void
async_receive_from(auto& socket,
                   asio::ip::udp::endpoint const& endpoint,
                   auto buffer,
                   Completion&& completion)
{
  socket.async_receive_from(
      buffer, endpoint, std::forward<Completion>(completion));
}

template<typename SocketType>
constexpr auto
create_bound_socket(asio::executor auto const& executor,
                    asio::ip::udp::endpoint const& endpoint)
{
  SocketType new_socket{ executor, endpoint.protocol() };

  if (endpoint.address().is_v6())
    new_socket.set_option(asio::ip::v6_only{ true });

  new_socket.bind(endpoint);

  return new_socket;
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
