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
#include <asio/execution/executor.hpp>
#include <asio/strand.hpp>
#include <asio/post.hpp>
#include <asio/defer.hpp>

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
template<asio::executor Executor, typename SocketType>
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
  auto async_join(std::span<endpoint const> initial_peers, Completion&& completion)
  {
    return async_join(my_id_, tracker_, routing_table_, initial_peers, std::forward<Completion>(completion));
  }

  template<typename Completion>
  auto async_stop(Completion&& completion) noexcept
  {
    auto stop = [this, completion=std::forward<Completion>(completion)] () mutable {
      network_.stop();
      asio::defer(executor_, std::move(completion));
    };

    asio::post(executor_, std::move(stop));
  }

  /**
   *
   */
  template<typename Completion>
  auto async_save(std::span<std::byte const> key,
                  std::span<std::byte const> data,
                  Completion&& completion)
  {
    return start_store_value_task(id(key),
                                  data,
                                  tracker_,
                                  routing_table_,
                                  std::forward<Completion>(completion));
  }

  /**
   *
   */
  template<typename Completion>
  auto async_load(std::span<std::byte const> key,
                  std::span<std::byte> buffer,
                  Completion&& completion)
  {
    return start_find_value_task<data_type>(
        id(key),
        tracker_,
        routing_table_,
        std::forward<Completion>(completion));
  }

private:
  ///
  using network_type = network<socket_type>;

  ///
  using random_engine_type = std::default_random_engine;

  ///
  using tracker_type = tracker<random_engine_type, network_type>;

  ///
  using endpoint_type = typename SocketType::endpoint;

  ///
  using routing_table_type = routing_table<endpoint_type>;

  ///
  using value_store_type = value_store<id, data_type>;


private:
  /**
   *
   */
  void process_new_message(ip_endpoint const& sender,
                           header const& h,
                           buffer::const_iterator i,
                           buffer::const_iterator e)
  {
    switch (h.type_) {
      case header::PING_REQUEST:
        handle_ping_request(sender, h);
        break;
      case header::STORE_REQUEST:
        handle_store_request(sender, h, i, e);
        break;
      case header::FIND_PEER_REQUEST:
        handle_find_peer_request(sender, h, i, e);
        break;
      case header::FIND_VALUE_REQUEST:
        handle_find_value_request(sender, h, i, e);
        break;
      default:
        tracker_.handle_new_response(sender, h, i, e);
        break;
    }
  }

  /**
   *
   */
  void handle_ping_request(ip_endpoint const& sender, header const& h)
  {
    tracker_.send_response(h.random_token_, header::PING_RESPONSE, sender);
  }

  /**
   *
   */
  void handle_store_request(ip_endpoint const& sender,
                            header const& h,
                            buffer::const_iterator i,
                            buffer::const_iterator e)
  {
    store_value_request_body request;
    if (auto failure = deserialize(i, e, request)) {
      return;
    }

    value_store_[request.data_key_hash_] = std::move(request.data_value_);
  }

  /**
   *
   */
  void handle_find_peer_request(ip_endpoint const& sender,
                                header const& h,
                                buffer::const_iterator i,
                                buffer::const_iterator e)
  {
    // Ensure the request is valid.
    find_peer_request_body request;
    if (auto failure = deserialize(i, e, request)) {
      return;
    }

    send_find_peer_response(sender, h.random_token_, request.peer_to_find_id_);
  }

  /**
   *
   */
  void send_find_peer_response(ip_endpoint const& sender,
                               id const& random_token,
                               id const& peer_to_find_id)
  {
    // Find X closest peers and save
    // their location into the response..
    find_peer_response_body response;

    auto remaining_peer = ROUTING_TABLE_BUCKET_SIZE;
    for (auto i = routing_table_.find(peer_to_find_id),
              e = routing_table_.end();
         i != e && remaining_peer > 0;
         ++i, --remaining_peer)
      response.peers_.push_back({ i->first, i->second });

    // Now send the response.
    tracker_.send_response(random_token, response, sender);
  }

  /**
   *
   */
  void handle_find_value_request(ip_endpoint const& sender,
                                 header const& h,
                                 buffer::const_iterator i,
                                 buffer::const_iterator e)
  {
    find_value_request_body request;
    if (auto failure = deserialize(i, e, request)) {
      return;
    }

    auto found = value_store_.find(request.value_to_find_);
    if (found == value_store_.end())
      send_find_peer_response(sender, h.random_token_, request.value_to_find_);
    else {
      find_value_response_body const response{ found->second };
      tracker_.send_response(h.random_token_, response, sender);
    }
  }

  /**
   *
   */
  void handle_new_message(ip_endpoint const& sender,
                          buffer::const_iterator i,
                          buffer::const_iterator e)
  {
    ks::dht::detail::header h;
    // Try to deserialize header.
    if (auto failure = deserialize(i, e, h)) {
      return;
    }

    routing_table_.push(h.source_id_, sender);

    process_new_message(sender, h, i, e);
  }

private:
  executor_type executor_;
  ///
  random_engine_type random_engine_{std::random_device{}()};
  ///
  id my_id_;
  ///
  network_type network_;
  ///
  tracker_type tracker_{executor_, my_id_, network_, random_engine_};
  ///
  routing_table_type routing_table_{my_id_};
  ///
  value_store_type value_store_{};
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
