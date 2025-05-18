// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>

#include <asio/wait_traits.hpp>
#include <asio/basic_waitable_timer.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename Executor>
using timer =
    asio::basic_waitable_timer<std::chrono::steady_clock,
                               asio::wait_traits<std::chrono::steady_clock>,
                               Executor>;

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
