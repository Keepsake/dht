// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include <asio/execution/executor.hpp>

#include "buffer.hpp"
#include "ip_endpoint.hpp"
#include "message_socket.hpp"

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

/**
 *
 */
template<typename SocketType>
class network final
{
public:
  ///
  using socket_type = SocketType;

  ///
  using endpoint_type = typename socket_type::ip_endpoint;

public:
  /**
   *
   */
  network(asio::executor Executor auto const& executor,
          endpoint_v4 const& endpoint_v4,
          endpoint_v6 const& endpoint_v6)
    : context_v4_{ .socket = create_socket(executor, endpoint_v4)}
    , context_v6_( .socket = create_socket(executor, endpoint_v6)}
  {
  }

  /**
   *
   */
  network(network const&) = delete;

  /**
   *
   */
  network& operator=(network const&) = delete;

  /**
   *
   */
  template<typename Completion>
  void async_send_to(endpoint_type const& endpoint,
                     std::span<std::byte const> buffer,
                     Completion&& completion)
  {
    get_socket_for(endpoint).async_send_to(
        asio::buffer(buffer), endpoint, std::forward<Completion>(completion));
  }

  /**
   *
   */
  template<typename Completion>
  auto async_receive_from_v4(endpoint_type & endpoint,
                             std::span<std::byte> buffer,
                             Completion&& completion)
  {
    async_receive_from(socket_v4_,
        endpoint, asio::buffer(buffer), std::forward<Completion>(completion));
  }

  /**
   *
   */
  template<typename Completion>
  auto async_receive_from_v6(endpoint_type & endpoint,
                             std::span<std::byte> buffer,
                             Completion&& completion)
  {
    async_receive_from(socket_v6_,
        endpoint, asio::buffer(buffer), std::forward<Completion>(completion));
  }

  void
  stop()
  {
    socket_v4_.close();
    socket_v6_.close();
  }

private:
  struct socket_context final
  {
    socket_type socket;
    endpoint_type sender_endpoint{}; 
    message_buffer receive_buffer{};
  };

private:
  constexpr auto create_socket(asio::executor auto const& executor,
                               auto endpoint)
  {
    return create_bound_socket<socket_type>(
        executor, endpoint_type{ endpoint.ip, endpoint.port });
  }

  /**
   *
   */
  socket_type& get_socket_for(endpoint_type const& e) noexcept
  {
    if (e.address_.is_v4())
      return socket_v4_;

    return socket_v6_;
  }

private:
  socket_context context_v4;
  socket_context context_v6;
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
