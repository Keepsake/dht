// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <queue>
#include <system_error>
#include <utility>
#include <variant>

#include <asio/bind_cancellation_slot.hpp>
#include <asio/cancellation_signal.hpp>

#include <ks/dht/detail/message.hpp>
#include <ks/dht/detail/scope_exit.hpp>
#include <ks/dht/endpoint.hpp>
#include <ks/dht/error.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename RequestTracker, typename RoutingTable, typename OnComplete>
class discover_neighbors_task final
  : public std::enable_shared_from_this<
        discover_neighbors_task<RequestTracker, RoutingTable, OnComplete>>
{
public:
  discover_neighbors_task(id my_id,
                          RequestTracker request_tracker,
                          RoutingTable routing_table,
                          std::queue<endpoint> endpoints_to_query,
                          OnComplete on_complete)
    : request_{ find_peer_request_body{ std::move(my_id) } }
    , request_tracker_{ std::forward<RequestTracker>(request_tracker) }
    , routing_table_{ std::forward<RoutingTable>(routing_table) }
    , endpoints_to_query_{ std::move(endpoints_to_query) }
    , on_complete_{ std::move(on_complete) }
  {
  }

  void search_myself()
  {
    if (endpoints_to_query_.empty()) {
      std::move(on_complete_)(
          make_error_code(error::initial_peer_failed_to_respond));
      return;
    }

    request_tracker_.send_request(
        endpoints_to_query_.front(),
        request_,
        response_,
        std::bind_front(&discover_neighbors_task::on_peer_response,
                        this->shared_from_this()));
  }

private:
  void on_peer_response(std::error_code failure)
  {
    endpoints_to_query_.pop();

    if (failure) {
      search_myself();
      return;
    }

    std::visit([this](auto body) { parse_response(std::move(body)); },
               std::move(response_));
  }

  void parse_response(find_peer_response_body body)
  {
    if (body.peers.empty()) {
      search_myself();
      return;
    }

    for (auto peer : std::move(body).peers)
      routing_table_.push(std::move(peer.id), std::move(peer.endpoint));

    std::move(on_complete_)(std::error_code{});
  }

  void parse_response(auto const& /* unknown body */) { search_myself(); }

private:
  message_body request_;
  RequestTracker request_tracker_;
  RoutingTable routing_table_;
  std::queue<endpoint> endpoints_to_query_;
  OnComplete on_complete_;
  message_body response_{};
};

template<typename RequestTracker, typename RoutingTable, typename OnComplete>
void
async_discover_neighbors(id const& my_id,
                         RequestTracker&& request_tracker,
                         RoutingTable&& routing_table,
                         std::queue<endpoint> endpoints_to_query,
                         OnComplete on_complete)
{
  using task =
      discover_neighbors_task<RequestTracker, RoutingTable, OnComplete>;

  std::make_shared<task>(my_id,
                         std::forward<RequestTracker>(request_tracker),
                         std::forward<RoutingTable>(routing_table),
                         std::move(endpoints_to_query),
                         std::move(on_complete))
      ->search_myself();
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
