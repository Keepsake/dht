// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

using clock = std::chrono::steady_clock;

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
