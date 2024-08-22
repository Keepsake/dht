// SPDX-License-Identifier: MIT

#pragma once

#include <asio/as_tuple.hpp>
#include <asio/use_awaitable.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename Executor>
using awaitable_token = asio::use_awaitable_t<Executor>;

template<typename Executor>
using nothrow_awaitable_token = asio::as_tuple_t<awaitable_token<Executor>>;

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
