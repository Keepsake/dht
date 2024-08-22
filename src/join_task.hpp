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

template<typename TrackerType,
         typename RoutingTableType,
         typename OnCompleteType>
class async_join final
{
public:
  async_join(id my_id,
             TrackerType& tracker,
             std::span<endpoint const> initial_peers)
    : my_id_(my_id)
    , tracker_(tacker)
    , initial_peers_(initial_peers)
  {
  }

  id my_id_;
  TrackerType& tracker_;
  std::vector<endpoint> initial_peers_;
};

constexpr auto
async_join(auto const& my_id,
           auto& tracker,
           auto& routing_table,
           auto const& endpoints_to_query,
           auto const& on_complete)
{
  return asio::async_initiate<Completion, void(std::error_code)>(
      std::move(init), std::forward<Completion>(completion), endpoints);
}

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
