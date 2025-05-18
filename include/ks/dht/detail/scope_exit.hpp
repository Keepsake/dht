// SPDX-License-Identifier: MIT

#pragma once

#include <optional>
#include <type_traits>
#include <utility>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

template<typename OnExit>
class scope_exit final
{
public:
  static constexpr bool noexcept_move =
      std::is_nothrow_move_constructible_v<OnExit>;

  static constexpr bool noexcept_invoke = std::is_nothrow_invocable_v<OnExit>;

public:
  [[nodiscard]] scope_exit(OnExit on_exit) noexcept(noexcept_move)
    : on_exit_{ std::move(on_exit) }
  {
  }

  ~scope_exit() noexcept(noexcept_invoke)
  {
    if (on_exit_)
      std::move(*on_exit_)();
  }

  scope_exit(scope_exit const& other) = delete;

  [[nodiscard]] scope_exit(scope_exit&& other) noexcept(noexcept_move)
    : on_exit_{ std::exchange(other.on_exit_, std::nullopt) }
  {
  }

  scope_exit& operator=(scope_exit&& other) noexcept(noexcept_move)
  {
    on_exit_ = std::exchange(other.on_exit_, std::nullopt);
  }

  scope_exit& operator=(scope_exit const& other) noexcept = delete;

private:
  std::optional<OnExit> on_exit_;
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
