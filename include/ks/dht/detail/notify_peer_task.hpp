// SPDX-License-Identifier: MIT
#pragma once

#include <list>
#include <memory>
#include <system_error>

#include <asio/bind_cancellation_slot.hpp>
#include <asio/cancellation_signal.hpp>

#include <ks/dht/detail/config.hpp>
#include <ks/dht/detail/message.hpp>
#include <ks/dht/detail/scope_exit.hpp>
#include <ks/dht/error.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename Candidates, typename RequestTracker, typename OnNotify>
class notify_peer_task final
  : public std::enable_shared_from_this<
        notify_peer_task<Candidates, RequestTracker, OnNotify>>
{
public:
  notify_peer_task(Candidates candidates,
                   RequestTracker request_tracker,
                   OnNotify on_notify)
    : candidates_{ std::forward<Candidates>(candidates) }
    , request_tracker_{ std::forward<RequestTracker>(request_tracker) }
    , on_notify_{ std::move(on_notify) }
  {
    setup_cancellation();
  }

  void notify_peers()
  {
    for (auto peer : candidates_.select_new_candidates(parallel_request_count))
      async_query_peer(std::move(peer));

    return_if_no_pending_query();
  }

private:
  struct query final
  {
    find_peer_request_body request{};
    message_body response{};
    asio::cancellation_signal cancellation_signal{};
  };

  using pending_queries = std::list<query>;

private:
  void setup_cancellation()
  {
    auto slot = asio::get_associated_cancellation_slot(on_notify_);
    if (slot.is_connected())
      slot.assign(std::bind_front(&notify_peer_task::cancel_all, this));
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
        find_peer_request_body{ candidates_.get_key() });

    request_tracker_.send_request(
        peer.endpoint,
        where->request,
        where->response,
        asio::bind_cancellation_slot(
            where->cancellation_signal.slot(),
            std::bind_front(&notify_peer_task::on_peer_response,
                            this->shared_from_this(),
                            where)));
  }

  void on_peer_response(pending_queries::iterator where,
                        std::error_code failure)
  {
    scope_exit on_exit{ [&] { pending_queries_.erase(where); } };

    if (failure) {
      return_if_no_pending_query();
      return;
    }

    std::visit([this](auto body) { parse_response(std::move(body)); },
               std::move(where->response));
  }

  void parse_response(find_peer_response_body body)
  {
    candidates_.add_candidates(std::move(body).peers);
    notify_peers();
  }

  void parse_response(auto const& /* unknown body */)
  {
    return_if_no_pending_query();
  }

  void return_if_no_pending_query()
  {
    if (this->weak_from_this().use_count() > 1U)
      return;

    std::move(on_notify_)();
  }

private:
  Candidates candidates_;
  RequestTracker request_tracker_;
  OnNotify on_notify_;
  pending_queries pending_queries_{};
};

template<typename Candidates, typename RequestTracker, typename OnNotify>
void
async_notify_peer(Candidates&& candidates,
                  RequestTracker&& request_tracker,
                  OnNotify on_notify)
{
  using task = notify_peer_task<Candidates, RequestTracker, OnNotify>;

  std::make_shared<task>(std::forward<Candidates>(candidates),
                         std::forward<RequestTracker>(request_tracker),
                         std::move(on_notify))
      ->notify_peers();
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
