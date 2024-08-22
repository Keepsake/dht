// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <functional>
#include <map>

#include <asio/basic_waitable_timer.hpp>
#include <asio/io_service.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

///
class timer final
{
public:
  ///
  using clock = std::chrono::steady_clock;

  ///
  using duration = clock::duration;

public:
  /**
   *
   */
  explicit timer(asio::io_service& io_service);

  /**
   *
   */
  template<typename Callback>
  void expires_from_now(duration const& timeout,
                        Callback const& on_timer_expired);

private:
  ///
  using time_point = clock::time_point;

  ///
  using callback = std::function<void(void)>;

  ///
  using timeouts = std::multimap<time_point, callback>;

  ///
  using deadline_timer = asio::basic_waitable_timer<clock>;

private:
  /**
   *
   */
  void schedule_next_tick(time_point const& expiration_time);

  /**
   *
   */
  void on_fire(std::error_code const& failure);

private:
  ///
  deadline_timer timer_;
  ///
  timeouts timeouts_;
};

template<typename Callback>
void
timer::expires_from_now(duration const& timeout,
                        Callback const& on_timer_expired)
{
  auto expiration_time = clock::now() + timeout;

  // If the current expiration time will be the sooner to expires
  // then cancel any pending wait and schedule this one instead.
  if (timeouts_.empty() || expiration_time < timeouts_.begin()->first)
    schedule_next_tick(expiration_time);

  timeouts_.emplace(expiration_time, on_timer_expired);
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
