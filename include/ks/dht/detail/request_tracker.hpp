// SPDX-License-Identifier: MIT

#pragma once

#include <cassert>
#include <map>
#include <random>
#include <system_error>
#include <utility>

#include <ks/dht/detail/completion_wrapper.hpp>
#include <ks/dht/detail/id.hpp>
#include <ks/dht/detail/message.hpp>
#include <ks/dht/detail/timer.hpp>
#include <ks/dht/endpoint.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename Executor, typename Network>
class request_tracker final
{
public:
  using executor_type = Executor;

public:
  explicit request_tracker(executor_type executor,
                           id const& my_id,
                           std::default_random_engine& random_engine,
                           Network& network,
                           std::chrono::milliseconds request_timeout)
    : executor_{ std::move(executor) }
    , my_id_{ my_id }
    , random_engine_{ random_engine }
    , network_{ network }
    , request_timeout_{ request_timeout }
  {
  }

  template<typename OnReceive>
  void send_request(endpoint const& endpoint,
                    message_body const& request_body,
                    message_body& response_body,
                    OnReceive on_receive)
  {
    id const random_token{ random_engine_ };

    auto const [where, inserted] = contexts_.emplace(
        random_token,
        context{ .timer{ executor_ },
                 .request_header{ .source_id{ my_id_ },
                                  .random_token{ random_token } },
                 .response_body{ &response_body },
                 .is_response_assigned{} });

    network_.async_send_to(
        endpoint,
        const_message_view{ .header{ where->second.request_header },
                            .body{ request_body } },
        on_send(where, std::move(on_receive)));
  }

  void handle_response(id const& request_token, message_body response_body)
  {
    auto const where = contexts_.find(request_token);
    if (where == contexts_.end()) [[unlikely]]
      return;

    auto& context = where->second;
    *context.response_body = std::move(response_body);
    context.is_response_assigned = true;
    context.timer.cancel();
  }

private:
  using timer_type = timer<executor_type>;

  struct context final
  {
    timer_type timer;
    message_header request_header;
    message_body* response_body;
    bool is_response_assigned;
  };

  using contexts = std::map<id, context>;

private:
  auto on_send(contexts::iterator context, auto on_receive)
  {
    return wrap_completion(
        [this, context](std::error_code failure, auto on_receive) {
          if (failure) [[unlikely]]
            finalize(context, failure, std::move(on_receive));
          else
            async_wait_for_response(context, std::move(on_receive));
        },
        std::move(on_receive));
  }

  void async_wait_for_response(contexts::iterator context, auto on_receive)
  {
    auto& timer = context->second.timer;
    timer.expires_after(request_timeout_);
    timer.async_wait(on_timeout(context, std::move(on_receive)));
  }

  auto on_timeout(contexts::iterator context, auto on_receive)
  {
    return wrap_completion(
        [this, context](std::error_code failure, auto on_receive) mutable {
          if (context->second.is_response_assigned)
            failure.clear();
          else if (not failure)
            failure = make_error_code(std::errc::timed_out);
          else
            failure = make_error_code(std::errc::operation_canceled);

          finalize(context, failure, std::move(on_receive));
        },
        std::move(on_receive));
  }

  void finalize(contexts::iterator context,
                std::error_code failure,
                auto on_receive)
  {
    contexts_.erase(context);
    std::move(on_receive)(failure);
  }

private:
  Executor executor_;
  id const& my_id_;
  std::default_random_engine& random_engine_;
  Network& network_;
  contexts contexts_{};
  std::chrono::milliseconds request_timeout_;
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
