// SPDX-License-Identifier: MIT

#pragma once

#include <asio/as_tuple.hpp>
#include <asio/this_coro.hpp>
#include <asio/use_awaitable.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename Executor>
using use_awaitable_t = asio::use_awaitable_t<Executor>;

template<typename Executor>
inline constexpr use_awaitable_t<Executor> use_awaitable{}; 

template<typename Executor>
using use_nothrow_awaitable_t = asio::as_tuple_t<use_awaitable_t<Executor>>;

template<typename Executor>
inline constexpr use_nothrow_awaitable_t<Executor> use_nothrow_awaitable{};

template<typename Executor>
using nothrow_awaitable_executor_t =
    use_nothrow_awaitable_t<Executor>::template executor_with_default<Executor>;

inline constexpr auto enable_partial_cancellation =
    asio::this_coro::reset_cancellation_state(
        asio::enable_partial_cancellation{});

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
