// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <asio/async_initiate.hpp>
#include <asio/defer.hpp>
#include <asio/execution/executor.hpp>
#include <asio/post.hpp>
#include <asio/strand.hpp>

#include <ks/dht/endpoint.hpp>

#include "discover_neighbors_task.hpp"
#include "error_impl.hpp"
#include "find_value_task.hpp"
#include "ip_endpoint.hpp"
#include "log.hpp"
#include "message.hpp"
#include "message_serializer.hpp"
#include "network.hpp"
#include "notify_peer_task.hpp"
#include "response_router.hpp"
#include "routing_table.hpp"
#include "store_value_task.hpp"
#include "tracker.hpp"
#include "value_store.hpp"

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

/**
 *
 */
template<typename Executor, template<typename, typename> typename Socket>
class engine final
{
public:
  ///
  using executor_type = asio::strand<Executor>;

  ///
  using socket_type = SocketType;

public:
  /**
   *
   */
  engine(executor_type executor,
         endpoint_v4 endpoint_v4,
         endpoint_v6 endpoint_v6,
         id const& new_id = id{})
    : executor_(std::move(executor))
    , my_id_(new_id == id{} ? id{ random_engine_ } : new_id)
    , network_(executor_, endpoint_v4, endpoint_v6)
  {
  }

  /**
   *
   */
  engine(engine const&) = delete;

  /**
   *
   */
  engine& operator=(engine const&) = delete;

  /**
   *
   */
  template<typename Completion>
  auto async_join(std::span<endpoint const> initial_peers,
                  Completion&& completion)
  {
    auto init = [this](auto handler, auto const& endpoints) {
      co_spawn(executor_,
               start_receiving<endpoint_v4>(),
               rethrow_exception);

      co_spawn(executor_,
               start_receiving<endpoint_v6>(),
               rethrow_exception);

      co_spawn(executor_,
               start_discover_neighbors_task(my_id_, tracker_, routing_table_, endpoints),
               forward_error(std::move(handler)));
    };

    return asio::async_initiate<Completion, void(std::error_code)>(
        std::move(init), std::forward<Completion>(completion), initial_peers);
  }

  template<typename Completion>
  auto async_stop(Completion&& completion) noexcept
  {
    auto init = [this](auto handler) {
      auto stop = [this, handler=std::move(handler)] mutable {
        network_.stop();
        std::move(handler)(std::error_code{});
      };

      asio::post(executor_, std::move(stop));
    };

    return asio::async_initiate<Completion, void(std::error_code)>(
        std::move(init), std::forward<Completion>(completion));
  }

  template<typename Completion>
  auto async_save(std::span<std::byte const> key,
                  std::span<std::byte const> data,
                  Completion&& completion)
  {
    auto init = [this](auto handler,
                       std::span<std::byte const> key,
                       std::span<std::byte const> data) {
      co_spawn(executor_,
               start_store_value_task(id{key},
                                      data,
                                      tracker_,
                                      routing_table_),
               forward_error(std::move(handler)));
    };

    return asio::async_initiate<Completion, void(std::error_code)>(
        std::move(init), std::forward<Completion>(completion), keys, data);
  }

  template<typename Completion>
  auto async_load(std::span<std::byte const> key,
                  std::span<std::byte> buffer,
                  Completion&& completion)
  {
    auto init = [this](auto handler,
                       std::span<std::byte const> key,
                       std::span<std::byte const> data) {
      co_spawn(executor_,
               start_find_value_task(id{key},
                                     data,
                                     tracker_,
                                     routing_table_),
               forward_error(std::move(handler)));
    };

    return asio::async_initiate<Completion, void(std::error_code)>(
        std::move(init), std::forward<Completion>(completion), keys, data);
  }

private:
  using network_type = network<socket_type>;
  using random_engine_type = std::default_random_engine;
  using tracker_type = tracker<random_engine_type, network_type>;
  using routing_table_type = routing_table<endpoint_type>;
  using value_store_type = value_store<id, data_type>;

private:
  void process_new_message(endpoint sender,
                           message & message)
  {
    routing_table_.push(message.header.source_id, sender);

    auto on_body = [&](auto & body) {
      handle_message(sender, message.header, body);
    };

    std::visit(std::move(on_body), message.body);
  }

  void handle_message(endpoint const& sender, message_header const& header, auto & body)
  {
    tracker_.handle_new_response(sender, header, body);
  }

  void handle_message(endpoint const& sender, message_header const& header, ping_request_body & /* body */)
  {
    tracker_.send_response(sender, header.random_token, ping_request_body{});
  }

  void handle_message(endpoint const& /* sender */, message_header const& /* header */, store_value_request_body & body)
  {
    value_store_[body.data_key_hash] = std::move(request.data_value);
  }

  void handle_message(endpoint const& sender,
                      message_header const& header,
                      find_peer_request_body & body)
  {
    send_find_peer_response(sender, header, body.peer_to_find);
  }

  void handle_message(endpoint const& sender,
                      message_header const& header,
                      find_value_request_body & body)
  {
    auto found = value_store_.find(request.value_to_find);
    if (found == value_store_.end())
      send_find_peer_response(sender, header, request.value_to_find);
    else
      tracker_.send_response(sender, header.random_token, find_peer_response_body{
        .data = found->second
      });
  }

  void send_find_peer_response(endpoint const& sender,
                      message_header const& header,
                      id const& id)
  {
    auto peers = std::ranges::subrange{routing_table_.find(id),
                                       routing_table_.end()}
          | std::views::take(ROUTING_TABLE_BUCKET_SIZE)
          | std::ranges::to<std::vector<peer>>();

    tracker_.send_response(sender, header.random_token, find_peer_response_body{
      .peers = std::move(peers),
    });
  }

  id get_closest_neighbor_id(void)
  {
    // Find our closest neighbor.
    auto closest_neighbor = routing_table_.find(my_id_);
    if (closest_neighbor->first == my_id_)
      ++closest_neighbor;

    assert(closest_neighbor != routing_table_.end() &&
           "at least one peer is known");

    return closest_neighbor->first;
  }

  template<typename OnInitialized>
  void notify_neighbors(OnInitialized on_initialized)
  {
    auto closest_neighbor_id = get_closest_neighbor_id();
    auto i = id::BIT_SIZE - 1;

    // Skip empty buckets.
    while (i && closest_neighbor_id[i] == my_id_[i])
      --i;

    pending_notifications_count_ += i;
    auto on_notification_complete = [this, on_initialized] {
      --pending_notifications_count_;

      if (!pending_notifications_count_)
        on_initialized();
    };

    // Send refresh from closest neighbor bucket to farest bucket.
    auto refresh_id = my_id_;
    while (i) {
      refresh_id[i] = !refresh_id[i];
      start_notify_peer_task(
          refresh_id, tracker_, routing_table_, on_notification_complete);
      --i;
    }
  }

  static constexpr auto forward_error(auto handler) noexcept
  {
    return [handler=std::move(handler)] mutable (std::exception_ptr e, std::error_code failure) {
      rethrow_exception(e);
      std::move(handler)(failure);
    };
  }

  template<typename Sender>
  asio::awaitable<void, executor_type>
  start_receiving()
  {
    Sender sender;
    detail::message message;
    for (;;) {
      auto const failure = co_await network_.async_receive_from(sender, message);
      if (failure) [[unlikely]]
        break;

      process_new_message(sender, message);
    }
  }

private:
  executor_type executor_;
  ///
  random_engine_type random_engine_{ std::random_device{}() };
  ///
  id my_id_;
  ///
  network_type network_;
  ///
  tracker_type tracker_{ executor_, my_id_, network_, random_engine_ };
  ///
  routing_table_type routing_table_{ my_id_ };
  ///
  value_store_type value_store_{};
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
