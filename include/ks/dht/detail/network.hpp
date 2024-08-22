// SPDX-License-Identifier: MIT

#pragma once

#include <ks/dht/detail/socket.hpp>
#include <ks/dht/endpoint.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

/**
 *
 */
template<typename Executor, template<typename, typename> Socket>
class network final
{
public:
  /**
   *
   */
  network(Executor const& executor,
          endpoint_v4 const& endpoint_v4,
          endpoint_v6 const& endpoint_v6)
    : socket_v4_{ executor, endpoint_v4 }
    , socket_v6_{ executor, endpoint_v6 }
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
  auto async_send_to(endpoint_v4 const& endpoint,
                     auto const& message)
  {
    return socket_v4_.async_send_to(endpoint, message);
  }

  /**
   *
   */
  auto async_send_to(endpoint_v6 const& endpoint,
                     auto const& message)
  {
    return socket_v6_.async_send_to(endpoint, message);
  }

  /**
   *
   */
  auto async_receive_from(endpoint_v4 & endpoint,
                          message & message)
  {
    return socket_v4_.async_receive_from(endpoint, message);
  }

  /**
   *
   */
  auto async_receive_from(endpoint_v6 & endpoint,
                          auto & message)
  {
    return socket_v6_.async_receive_from(endpoint, message);
  }

  void stop()
  {
    socket_v4_.close();
    socket_v6_.close();
  }

private:
  Socket<Executor, endpoint_v4> socket_v4_;
  Socket<Executor, endpoint_v6> socket_v6_;
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
