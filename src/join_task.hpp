// SPDX-License-Identifier: MIT

#pragma once

#include <span>
#include <cstddef>
#include <system_error>

#include <ks/dht/error.hpp>

#include "constants.hpp"
#include "message.hpp"

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename TrackerType,
         typename RoutingTableType,
         typename EndpointsType>
struct join_task final
{
  id const& my_id_;
  TrackerType & tracker_;
  RoutingTableType & routing_table_;
  EndpointsType endpoints_to_query_;
  
  void operator()(auto & self)
  {
      search_ourselves(self);
  }

  void operator()(auto &self,
                  std::error_code failure,
                  ip_endpoint const& /* from */,
                  header const& header,
                  std::span<std::byte const> payload)
  {
    if (failure) {
      search_ourselves(self);
      return;
    }

    handle_initial_contact_response(self, header, payload);
  }

  void search_ourselves(auto & self)
  {
    if (endpoints_to_query_.empty()) {
      self.complete(make_error_code(INITIAL_PEER_FAILED_TO_RESPOND));
      return;
    }

    // Retrieve the next endpoint to query.
    auto const endpoint_to_query = endpoints_to_query_.back();
    endpoints_to_query_.pop_back();

    tracker_.send_request(find_peer_request_body{ my_id_ },
                          endpoint_to_query,
                          INITIAL_CONTACT_RECEIVE_TIMEOUT,
                          std::move(self));
  }

  void handle_initial_contact_response(auto & self,
                                       header const& header,
                                       std::span<std::byte const> payload)
  {
    if (header.type_ != header::FIND_PEER_RESPONSE) {
      search_ourselves(self);
      return;
    };

    find_peer_response_body response;
    if (const auto failure = deserialize(payload, response); failure) {
      search_ourselves(self);
      return;
    }

    // Add discovered peers.
    for (auto const& peer : response.peers_)
      routing_table_.push(peer.id_, peer.endpoint_);

    notify_neighbors(self);
  }

  /**
   *  Refresh each bucket.
   */
  void notify_neighbors(auto & self)
  {
    auto closest_neighbor_id = get_closest_neighbor_id();
    auto i = id::BIT_SIZE - 1;

    // Skip empty buckets.
    while (i and closest_neighbor_id[i] == my_id_[i])
      --i;

    // Send refresh from closest neighbor bucket to farest bucket.
    auto refresh_id = my_id_;
    while (i) {
      refresh_id[i] = !refresh_id[i];
      start_notify_peer_task(refresh_id, tracker_, routing_table_);
      --i;
    }

    self.complete(std::error_code{});
  }

  /**
   *
   */
  id get_closest_neighbor_id()
  {
    // Find our closest neighbor.
    auto closest_neighbor = routing_table_.find(my_id_);
    if (closest_neighbor->first == my_id_)
      ++closest_neighbor;

    assert(closest_neighbor != routing_table_.end() &&
           "at least one peer is known");

    return closest_neighbor->first;
  }
};

template<typename Completion>
auto
async_join(id const& my_id,
           auto & tracker,
           auto & routing_table,
           auto const& endpoints_to_query,
           Completion && completion)
{
  join_task task{my_id, tracker, routing_table, endpoints_to_query};

  return asio::async_compose<Completion, void(std::error_code)>
      (std::move(task), std::forward<Completion>(completion));
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
