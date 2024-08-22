// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <system_error>
#include <type_traits>

#include "error_impl.hpp"

#include "constants.hpp"
#include "log.hpp"
#include "message.hpp"

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

///
template<typename TrackerType,
         typename RoutingTableType,
         typename EndpointsType,
         typename OnCompleteType>
class discover_neighbors_task final
{
public:
  ///
  using tracker_type = TrackerType;
  ///
  using endpoints_type = EndpointsType;
  ///
  using routing_table_type = RoutingTableType;
  ///
  using on_complete_type = OnCompleteType;

public:
  /**
   *
   */
  discover_neighbors_task(id const& my_id,
                          tracker_type& tracker,
                          routing_table_type& routing_table,
                          endpoints_type const& endpoints_to_query,
                          on_complete_type const& on_complete)
    : my_id_(my_id)
    , tracker_(tracker)
    , routing_table_(routing_table)
    , endpoints_to_query_(endpoints_to_query)
    , on_complete_(on_complete)
  {
    search_ourselves();
  }

private:
  /**
   *
   */
  void search_ourselves()
  {
    if (task->endpoints_to_query_.empty()) {
      auto const f = make_error_code(INITIAL_PEER_FAILED_TO_RESPOND);
      std::move(on_complete_)(f);
      return;
    }

    // Retrieve the next endpoint to query.
    auto const endpoint_to_query = endpoints_to_query_.back();
    endpoints_to_query_.pop_back();

    // On message received, process it.
    auto on_message_received = [this](ip_endpoint const& s,
                                      header const& h,
                                      buffer::const_iterator i,
                                      buffer::const_iterator e) {
      handle_initial_contact_response(s, h, i, e);
    };

    // On error, retry with another endpoint.
    auto on_error = [task](std::error_code const&) { search_ourselves(task); };

    tracker_.send_request(find_peer_request_body{ task->my_id_ },
                          endpoint_to_query,
                          INITIAL_CONTACT_RECEIVE_TIMEOUT,
                          on_message_received,
                          on_error);
  }

  /**
   *
   */
  void handle_initial_contact_response(ip_endpoint const& s,
                                       header const& h,
                                       buffer::const_iterator i,
                                       buffer::const_iterator e)
  {
    if (h.type_ != header::FIND_PEER_RESPONSE) {
      search_ourselves();
      return;
    };

    find_peer_response_body response;
    if (auto failure = deserialize(i, e, response)) {
      search_ourselves();
      return;
    }

    // Add discovered peers.
    for (auto const& peer : response.peers_)
      routing_table_.push(peer.id_, peer.endpoint_);

    std::move(on_complete_)(std::error_code{});
  }

private:
  ///
  id const& my_id_;
  ///
  tracker_type& tracker_;
  ///
  routing_table_type& routing_table_;
  ///
  endpoints_type endpoints_to_query_;
  ///
  on_complete_type on_complete_;
};

/**
 *
 */
template<typename TrackerType,
         typename RoutingTableType,
         typename EndpointsType,
         typename OnCompleteType>
void
start_discover_neighbors_task(id const& my_id,
                              TrackerType& tracker,
                              RoutingTableType& routing_table,
                              EndpointsType const& endpoints_to_query,
                              OnCompleteType const& on_complete)
{
  using task = discover_neighbors_task<TrackerType,
                                       RoutingTableType,
                                       EndpointsType,
                                       OnCompleteType>;

  task::start(my_id, tracker, routing_table, endpoints_to_query, on_complete);
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
