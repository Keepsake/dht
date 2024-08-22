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
struct discover_neighbors_task final
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

    self.complete(std::error_code{});
  }
};

template<typename Completion>
auto
start_discover_neighbors_task(id const& my_id,
                              auto & tracker,
                              auto & routing_table,
                              auto const& endpoints_to_query,
                              Completion && completion)
{
  discover_neighbors_task task{my_id, tracker, routing_table, endpoints_to_query};

  return asio::async_compose<Completion, void(std::error_code)>
      (std::move(task), std::forward<Completion>(completion));
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
