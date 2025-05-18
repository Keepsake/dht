// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <list>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include <asio/bind_cancellation_slot.hpp>
#include <asio/cancellation_signal.hpp>

#include <ks/dht/detail/config.hpp>
#include <ks/dht/detail/message.hpp>
#include <ks/dht/detail/scope_exit.hpp>
#include <ks/dht/error.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

using find_value_data = std::vector<std::byte>;

template<typename Candidates, typename RequestTracker, typename OnFind>
class find_value_task final
  : public std::enable_shared_from_this<
        find_value_task<Candidates, RequestTracker, OnFind>>
{
public:
  find_value_task(Candidates candidates,
                  RequestTracker request_tracker,
                  OnFind on_find)
    : candidates_{ std::forward<Candidates>(candidates) }
    , request_tracker_{ std::forward<RequestTracker>(request_tracker) }
    , on_find_{ std::move(on_find) }
  {
    setup_cancellation();
  }

  void query_new_candidates()
  {
    for (auto peer : candidates_.select_new_candidates(parallel_request_count))
      async_query_peer(std::move(peer));

    return_value_not_found_if_no_pending_query();
  }

private:
  struct query final
  {
    find_value_request_body request{};
    message_body response{};
    asio::cancellation_signal cancellation_signal{};
  };

  using pending_queries = std::list<query>;

private:
  void setup_cancellation()
  {
    auto slot = asio::get_associated_cancellation_slot(on_find_);
    if (slot.is_connected())
      slot.assign(std::bind_front(&find_value_task::cancel_all, this));
  }

  void cancel_all(asio::cancellation_type type)
  {
    for (auto& query : pending_queries_)
      query.cancellation_signal.emit(type);
  }

  void async_query_peer(peer peer)
  {
    auto where = pending_queries_.emplace(
        pending_queries_.end(),
        find_value_request_body{ candidates_.get_key() });

    request_tracker_.send_request(
        peer.endpoint,
        where->request,
        where->response,
        asio::bind_cancellation_slot(
            where->cancellation_signal.slot(),
            std::bind_front(&find_value_task::on_peer_response,
                            this->shared_from_this(),
                            where)));
  }

  void on_peer_response(pending_queries::iterator where,
                        std::error_code failure)
  {
    scope_exit on_exit{ [&] { pending_queries_.erase(where); } };

    if (is_value_found_)
      return;

    if (failure) {
      return_value_not_found_if_no_pending_query();
      return;
    }

    std::visit([this](auto body) { parse_response(std::move(body)); },
               std::move(where->response));
  }

  void parse_response(find_value_response_body body)
  {
    return_data(std::move(body.data));
  }

  void parse_response(find_peer_response_body body)
  {
    candidates_.add_candidates(std::move(body).peers);
    query_new_candidates();
  }

  void parse_response(auto const& /* unknown body */)
  {
    return_value_not_found_if_no_pending_query();
  }

  void return_data(find_value_data data)
  {
    std::move(on_find_)(std::error_code{}, std::move(data));
    is_value_found_ = true;
  }

  void return_value_not_found_if_no_pending_query()
  {
    if (this->weak_from_this().use_count() > 1U)
      return;

    std::move(on_find_)(make_error_code(error::value_not_found),
                        find_value_data{});
  }

private:
  Candidates candidates_;
  RequestTracker request_tracker_;
  OnFind on_find_;
  pending_queries pending_queries_{};
  bool is_value_found_{};
};

template<typename Candidates, typename RequestTracker, typename OnFind>
void
async_find_value(Candidates&& candidates,
                 RequestTracker&& request_tracker,
                 OnFind on_find)
{
  using task = find_value_task<Candidates, RequestTracker, OnFind>;
  std::make_shared<task>(std::forward<Candidates>(candidates),
                         std::forward<RequestTracker>(request_tracker),
                         std::move(on_find))
      ->query_new_candidates();
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
