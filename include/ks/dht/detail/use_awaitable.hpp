// SPDX-License-Identifier: MIT

#pragma once

#include <asio/as_tuple.hpp>
#include <asio/use_awaitable.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename Executor>
using awaitable_executor = asio::as_tuple_t<
    asio::use_awaitable_t<Executor>>::template executor_with_default<Executor>;

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
