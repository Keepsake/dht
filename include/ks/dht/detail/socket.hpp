// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <system_error>
#include <type_traits>

#include <asio/basic_datagram_socket.hpp>
#include <asio/ip/udp.hpp>
#include <asio/ip/v6_only.hpp>

#include <ks/serialization/serialize.hpp>

#include <ks/dht/detail/completion_wrapper.hpp>
#include <ks/dht/endpoint.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename Executor, typename Endpoint>
class socket final
{
public:
  using executor_type = Executor;

  using endpoint_type = Endpoint;

public:
  explicit socket(executor_type const& executor, endpoint_type const& endpoint)
    : socket_{ create_socket(executor, endpoint) }
  {
  }

  socket(socket const&) = delete;

  socket& operator=(socket const&) = delete;

  endpoint_type get_local_endpoint() const
  {
    return from_udp_endpoint(socket_.local_endpoint());
  }

  template<typename OnSend>
  void async_send_to(endpoint_type const& endpoint,
                     auto const& message,
                     OnSend on_send)
  {
    if (auto const failure = serialize_message(message); failure) [[unlikely]] {
      std::move(on_send)(failure);
      return;
    }

    auto token = wrap_completion(
        [](std::error_code failure, std::size_t, auto on_send) {
          std::move(on_send)(failure);
        },
        std::move(on_send));

    socket_.async_send_to(asio::buffer(send_buffer_),
                          to_udp_endpoint(endpoint),
                          std::move(token));
  }

  template<typename OnReceive>
  void async_receive_from(endpoint_type& endpoint,
                          auto& message,
                          OnReceive on_receive)
  {
    auto token = wrap_completion(
        [&](std::error_code failure, std::size_t size, auto on_receive) {
          if (not failure) [[likely]] {
            endpoint = from_udp_endpoint(receive_endpoint_);
            failure = ks::serialization::load(
                std::span{ receive_buffer_.get(), size }, message);
          }

          std::move(on_receive)(failure);
        },
        std::move(on_receive));

    socket_.async_receive_from(asio::buffer(receive_buffer_.get(), buffer_size),
                               receive_endpoint_,
                               std::move(token));
  }

  constexpr void close() { socket_.close(); }

private:
  static constexpr std::size_t buffer_size =
      std::numeric_limits<std::uint16_t>::max();

  using socket_type = asio::basic_datagram_socket<asio::ip::udp, executor_type>;

  using socket_endpoint = socket_type::endpoint_type;

private:
  std::error_code serialize_message(auto const& message)
  {
    send_buffer_.clear();
    return ks::serialization::save(send_buffer_, message);
  }

  static socket_endpoint to_udp_endpoint(endpoint_type const& endpoint)
  {
    return { endpoint.ip, endpoint.port };
  }

  static endpoint_type from_udp_endpoint(socket_endpoint const& endpoint)
  {
    if constexpr (std::is_same_v<endpoint_type, endpoint_v4>)
      return { endpoint.address().to_v4(), endpoint.port() };
    else
      return { endpoint.address().to_v6(), endpoint.port() };
  }

  static constexpr auto create_socket(executor_type const& executor,
                                      auto const& endpoint)
  {
    auto const udp_endpoint = to_udp_endpoint(endpoint);

    socket_type new_socket{ executor, udp_endpoint.protocol() };

    if constexpr (std::is_same_v<endpoint_type, endpoint_v6>)
      new_socket.set_option(asio::ip::v6_only{ true });

    new_socket.bind(udp_endpoint);

    return new_socket;
  }

private:
  socket_type socket_;
  socket_endpoint receive_endpoint_{};
  std::unique_ptr<std::byte[]> receive_buffer_{
    std::make_unique_for_overwrite<std::byte[]>(buffer_size)
  };
  std::vector<std::byte> send_buffer_{};
};

template<typename Executor>
using socket_v4 = socket<Executor, endpoint_v4>;

template<typename Executor>
using socket_v6 = socket<Executor, endpoint_v6>;

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
