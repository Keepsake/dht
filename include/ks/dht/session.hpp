// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <span>
#include <utility>
#include <variant>

#include <asio/basic_datagram_socket.hpp>
#include <asio/execution/executor.hpp>
#include <asio/ip/udp.hpp>

#include <ks/dht/detail/engine.hpp>
#include <ks/dht/detail/symbol_visibility.hpp>
#include <ks/dht/endpoint.hpp>

namespace ks::dht {
inline namespace abiv1 {

template<asio::executor Executor>
class session final
{
public:
  using executor_type = Executor;

  template<asio::executor NewExecutor>
  struct rebind_executor final
  {
    using other = session<NewExecutor>;
  };

public:
  session(executor_type executor,
          endpoint_v4 const& endpoint_v4,
          endpoint_v6 const& endpoint_v6)
    : engine_(std::move(executor), endpoint_v4, endpoint_v6)
  {
  }

  session(session const&) = delete;

  session& operator=(session const&) = delete;

  template<typename Completion>
  auto async_join(std::span<endpoint const> initial_peers, Completion&& completion)
  {
    return engine_.async_join(initial_peers, std::forward<Completion>(completion));
  }

  template<typename Completion>
  auto async_stop(Completion&& completion)
  {
    return engine_.stop(std::forward<Completion>(completion));
  }

  template<typename Completion>
  auto async_save(std::span<std::byte const> key,
                  std::span<std::byte const> data,
                  Completion&& completion)
  {
    return engine_.async_save(key, data, std::forward<Completion>(completion));
  }

  template<typename Completion>
  auto async_load(std::span<std::byte const> key,
                  std::span<std::byte> buffer,
                  Completion&& completion)
  {
    return engine_.async_load(
        key, buffer, std::forward<Completion>(completion));
  }

private:
  using socket_type = asio::basic_datagram_socket<asio::ip::udp, executor_type>;

  using engine_type = detail::engine<executor_type, socket_type>;

private:
  engine_type engine_;
};

} // namespace abiv1
} // namespace ks::dht
