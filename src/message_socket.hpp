// SPDX-License-Identifier: MIT

#pragma once

#include <utility>
#ifdef _MSC_VER
# include <system_error>
#endif

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

#ifdef _MSC_VER

template<typename OnComplete>
constexpr void
async_receive_from(auto& socket,
                   auto& endpoint,
                   auto& message,
                   OnComplete on_complete)
{
  auto on_receive =
      [&socket, &endpoint, &message, on_complete = std::move(on_complete)](
          std::error_code failure) mutable {
        // On Windows, an UDP socket may return connection_reset
        // to inform application that a previous send by this socket
        // has generated an ICMP port unreachable.
        // https://msdn.microsoft.com/en-us/library/ms740120.aspx
        // Ignore it and schedule another read.
        if (failure == std::errc::connection_reset) {
          async_receive_from(socket, endpoint, message, std::move(on_complete));
          return;
        }

        std::move(on_complete)(failure);
      };

  socket.async_receive_from(endpoint, message, std::move(on_receive));
}

#else

template<typename OnReceive>
constexpr void
async_receive_from(auto& socket,
                   auto& endpoint,
                   auto& message,
                   OnReceive on_receive)
{
  socket.async_receive_from(endpoint, message, std::move(on_receive));
}

#endif

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
