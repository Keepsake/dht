// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <queue>
#include <system_error>
#include <type_traits>

#include <asio/as_tuple.hpp>
#include <asio/basic_datagram_socket.hpp>
#include <asio/ip/udp.hpp>
#include <asio/ip/v6_only.hpp>
#include <asio/use_awaitable.hpp>

#include <ks/serialization/serialize.hpp>

#include <ks/dht/detail/awaitable.hpp>
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
  socket(executor_type const& executor, endpoint_type const& endpoint)
    : socket_(create_socket(executor, endpoint))
  {
  }

  socket(socket const&) = delete;

  socket& operator=(socket const&) = delete;

  endpoint_type get_local_endpoint() const
  {
    return from_udp_endpoint(socket_.local_endpoint());
  }

  auto async_send_to(endpoint_type const& endpoint, auto const& message)
  {
    auto init = [&](auto handler) {
      init_send([&,
                 work = asio::make_work_guard(handler),
                 handler = std::move(handler)] mutable {
        if (auto failure = serialize_message(message); failure) [[unlikely]] {
          finalize_send();
          std::move(handler)(failure);
          return;
        }

        socket_.async_send_to(
            asio::buffer(send_buffer_),
            to_udp_endpoint(endpoint),
            [this, work = std::move(work), handler = std::move(handler)](
                std::error_code failure, std::size_t) mutable {
              finalize_send();
              std::move(handler)(failure);
            });
      });
    };

    return asio::async_initiate<void(std::error_code)>(std::move(init), token);
  }

  auto async_receive_from(endpoint_type& endpoint, auto& message)
  {
    auto init = [&](auto handler) {
      socket_.async_receive_from(
          asio::buffer(receive_buffer_.get(), buffer_size),
          receive_endpoint_,
          [&,
           work = asio::make_work_guard(handler),
           handler = std::move(handler)](std::error_code failure,
                                         std::size_t size) mutable {
            if (not failure) [[likely]] {
              endpoint = from_udp_endpoint(receive_endpoint_);
              failure = ks::serialization::load(
                  std::span{ receive_buffer_.get(), size }, message);
            }

            std::move(handler)(failure);
          });
    };

    return asio::async_initiate<void(std::error_code)>(std::move(init), token);
  }

  constexpr void close() { socket_.close(); }

private:
  static constexpr std::size_t buffer_size =
      std::numeric_limits<std::uint16_t>::max();

  using socket_type = asio::basic_datagram_socket<asio::ip::udp, Executor>;

  static constexpr nothrow_awaitable_token<executor_type> token{};

  using socket_endpoint = socket_type::endpoint_type;

private:
  // std::move_only_function not yet available on clang
  struct pending_send_base
  {
    virtual ~pending_send_base() noexcept = default;
    virtual void schedule() = 0;
  };

  template<typename Send>
  class pending_send final : public pending_send_base
  {
  public:
    pending_send(Send send)
      : send_(std::move(send))
    {
    }

    void schedule() override { std::move(send_)(); }

  private:
    Send send_;
  };

private:
  std::error_code serialize_message(auto const& message)
  {
    send_buffer_.clear();
    return ks::serialization::save(send_buffer_, message);
  }

  template<typename Send>
  void init_send(Send&& send)
  {
    pending_send_.push(
        std::make_unique<pending_send<Send>>(std::forward<Send>(send)));
    if (pending_send_.size() == 1U)
      init_send();
  }

  void init_send()
  {
    assert(not pending_send_.empty());
    auto pending_send = std::move(pending_send_.front());
    pending_send->schedule();
  }

  void finalize_send()
  {
    assert(not pending_send_.empty());
    pending_send_.pop();
    if (not pending_send_.empty())
      init_send();
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
  socket_endpoint receive_endpoint_;
  std::unique_ptr<std::byte[]> receive_buffer_{
    std::make_unique_for_overwrite<std::byte[]>(buffer_size)
  };
  std::vector<std::byte> send_buffer_{};
  std::queue<std::unique_ptr<pending_send_base>> pending_send_{};
};

template<typename Executor>
using socket_v4 = socket<Executor, endpoint_v4>;

template<typename Executor>
using socket_v6 = socket<Executor, endpoint_v6>;

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
