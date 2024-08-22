// SPDX-License-Identifier: MIT

#pragma once

#include <asio/as_tuple.hpp>
#include <asio/use_awaitable.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

inline void
rethrow_exception(std::exception_ptr ptr)
{
  if (ptr)
    std::rethrow_exception(ptr);
}

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
