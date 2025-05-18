// SPDX-License-Identifier: MIT

#pragma once

#include <utility>
#include <variant>

#include <ks/dht/endpoint.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename SocketV4, typename SocketV6>
class network final
{
public:
  explicit network(SocketV4 socket_v4, SocketV6 socket_v6)
    : socket_v4_{ std::forward<SocketV4>(socket_v4) }
    , socket_v6_{ std::forward<SocketV6>(socket_v6) }
  {
  }

  template<typename CompletionToken>
  void async_send_to(endpoint const& endpoint,
                     auto const& message,
                     CompletionToken token)
  {
    std::visit(
        [&](auto const& e) { async_send_to(e, message, std::move(token)); },
        endpoint);
  }

  template<typename CompletionToken>
  void async_send_to(endpoint_v4 const& endpoint,
                     auto const& message,
                     CompletionToken token)
  {
    socket_v4_.async_send_to(endpoint, message, std::move(token));
  }

  template<typename CompletionToken>
  void async_send_to(endpoint_v6 const& endpoint,
                     auto const& message,
                     CompletionToken token)
  {
    socket_v6_.async_send_to(endpoint, message, std::move(token));
  }

  template<typename CompletionToken>
  void async_receive_from(endpoint_v4& endpoint,
                          auto& message,
                          CompletionToken token)
  {
    socket_v4_.async_receive_from(endpoint, message, std::move(token));
  }

  template<typename CompletionToken>
  void async_receive_from(endpoint_v6& endpoint,
                          auto& message,
                          CompletionToken token)
  {
    socket_v6_.async_receive_from(endpoint, message, std::move(token));
  }

  void stop()
  {
    socket_v4_.close();
    socket_v6_.close();
  }

private:
  SocketV4 socket_v4_;
  SocketV6 socket_v6_;
};

template<typename SocketV4, typename SocketV6>
network(SocketV4&&, SocketV6&&) -> network<SocketV4, SocketV6>;

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
