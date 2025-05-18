// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <ks/dht/detail/async_queue.hpp>
#include <ks/dht/detail/socket.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename UnderlyingSocket>
class concurrent_socket final
{
public:
  using socket_type = UnderlyingSocket;

  using endpoint_type = typename UnderlyingSocket::endpoint_type;

  using executor_type = typename UnderlyingSocket::executor_type;

public:
  template<typename... Args>
  explicit concurrent_socket(Args&&... args)
    : socket_{ std::forward<Args>(args)... }
  {
  }

  concurrent_socket(concurrent_socket const&) = delete;

  concurrent_socket& operator=(concurrent_socket const&) = delete;

  endpoint_type get_local_endpoint() const
  {
    return socket_.get_local_endpoint();
  }

  template<typename OnComplete>
  void async_send_to(endpoint_type const& endpoint,
                     auto const& message,
                     OnComplete on_complete)
  {
    auto init = [this, endpoint, &message](auto on_complete) mutable {
      socket_.async_send_to(endpoint, message, std::move(on_complete));
    };

    queue_.queue(std::move(init), std::move(on_complete));
  }

  template<typename OnComplete>
  void async_receive_from(endpoint_type& endpoint,
                          auto& message,
                          OnComplete&& on_complete)
  {
    socket_.async_receive_from(
        endpoint, message, std::forward<OnComplete>(on_complete));
  }

  constexpr void close() { socket_.close(); }

private:
  socket_type socket_;
  async_queue queue_{};
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
