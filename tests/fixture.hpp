// SPDX-License-Identifier: MIT

#include <utility>

#include <gtest/gtest.h>

#include <asio/async_result.hpp>
#include <asio/awaitable.hpp>
#include <asio/io_context.hpp>

#include <ks/dht/detail/awaitable.hpp>

class io_fixture : public testing::Test
{
protected:
  using executor_type = asio::io_context::executor_type;

  using use_awaitable_type =
      ks::dht::detail::use_nothrow_awaitable_t<executor_type>;

  template<typename Return>
  using awaitable = asio::awaitable<Return, executor_type>;

protected:
  template<typename... Args>
  constexpr auto as_awaitable(auto init)
  {
    use_awaitable_type token{};
    return asio::async_initiate<use_awaitable_type, void(Args...)>(
        std::move(init), token);
  }

protected:
  asio::io_context io_context_{};
  asio::io_context::executor_type executor_{ io_context_.get_executor() };
};
