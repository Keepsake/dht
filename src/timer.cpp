// SPDX-License-Identifier: MIT

#include "timer.hpp"

#include <functional>

#include "error_impl.hpp"
#include "log.hpp"

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

timer::timer(asio::io_service& io_service)
  : timer_{ io_service }
  , timeouts_{}
{
}

void
timer::schedule_next_tick(time_point const& expiration_time)
{
  // This will cancel any pending task.
  timer_.expires_at(expiration_time);

  LOG_DEBUG(timer, this) << "schedule callback at "
                         << expiration_time.time_since_epoch().count() << "."
                         << std::endl;

  using std::placeholders::_1;
  timer_.async_wait(std::bind(&timer::on_fire, this, _1));
}

void
timer::on_fire(system::error_code const& failure)
{
  // The current timeout has been canceled
  // hence stop right there.
  if (failure == asio::error::operation_aborted)
    return;

  if (failure)
    throw std::system_error{ make_error_code(TIMER_MALFUNCTION) };

  // The callbacks to execute are the first
  // n callbacks with the same keys.
  auto begin = timeouts_.begin();
  auto end = timeouts_.upper_bound(begin->first);
  // Call the user callbacks.
  for (auto i = begin; i != end; ++i)
    i->second();

  LOG_DEBUG(timer, this) << "remove " << std::distance(begin, end)
                         << " callback(s) scheduled at "
                         << begin->first.time_since_epoch().count() << "."
                         << std::endl;

  // And remove the timeout.
  timeouts_.erase(begin, end);

  // If there is a remaining timeout, schedule it.
  if (!timeouts_.empty()) {
    LOG_DEBUG(timer, this) << "schedule remaining timers" << std::endl;
    schedule_next_tick(timeouts_.begin()->first);
  }
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
