// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <string>

#include <asio/as_tuple.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>

#include <ks/dht/detail/awaitable.hpp>
#include <ks/dht/detail/rethrow_exception.hpp>
#include <ks/dht/detail/socket.hpp>
#include <ks/dht/endpoint.hpp>

#include "fixture.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

class test_socket : public io_fixture
{
protected:
  auto as_awaitable(auto init)
  {
    return io_fixture::as_awaitable<std::error_code>(std::move(init));
  }
};

TEST_F(test_socket, can_socket_v4_loopback)
{
  kd::socket sender{
    executor_, k::endpoint_v4{ asio::ip::make_address_v4("127.0.0.1"), 0 }
  };

  kd::socket receiver{
    executor_, k::endpoint_v4{ asio::ip::make_address_v4("127.0.0.1"), 0 }
  };

  auto const local_endpoint = receiver.get_local_endpoint();
  std::string const expected("data");
  k::endpoint_v4 sender_endpoint;
  std::string actual;

  auto test = [&] -> awaitable<void> {
    co_await as_awaitable([&](auto token) {
      sender.async_send_to(local_endpoint, expected, std::move(token));
    });

    co_await as_awaitable([&](auto token) {
      receiver.async_receive_from(sender_endpoint, actual, std::move(token));
    });
  };

  co_spawn(executor_, test, kd::rethrow_non_null_exception);
  io_context_.run();

  EXPECT_EQ(expected, actual);
  EXPECT_EQ(sender_endpoint, sender.get_local_endpoint());
}

TEST_F(test_socket, can_socket_v6_loopback)
{
  kd::socket sender{ executor_,
                     k::endpoint_v6{ asio::ip::make_address_v6("::1"), 0 } };

  kd::socket receiver{ executor_,
                       k::endpoint_v6{ asio::ip::make_address_v6("::1"), 0 } };

  auto const local_endpoint = receiver.get_local_endpoint();
  std::string const expected("data");
  k::endpoint_v6 sender_endpoint;
  std::string actual;

  auto test = [&] -> awaitable<void> {
    co_await as_awaitable([&](auto token) {
      sender.async_send_to(local_endpoint, expected, std::move(token));
    });

    co_await as_awaitable([&](auto token) {
      receiver.async_receive_from(sender_endpoint, actual, std::move(token));
    });
  };

  co_spawn(executor_, test, kd::rethrow_non_null_exception);
  io_context_.run();

  EXPECT_EQ(expected, actual);
  EXPECT_EQ(sender_endpoint, sender.get_local_endpoint());
}

TEST_F(test_socket, can_detect_close)
{
  kd::socket sender{ executor_,
                     k::endpoint_v6{ asio::ip::make_address_v6("::1"), 0 } };

  sender.close();

  kd::socket receiver{ executor_,
                       k::endpoint_v6{ asio::ip::make_address_v6("::1"), 0 } };

  auto const local_endpoint = receiver.get_local_endpoint();
  std::size_t called{};
  std::string const expected("data");

  auto on_send = [&](std::error_code failure) {
    EXPECT_TRUE(failure);
    ++called;
  };

  sender.async_send_to(local_endpoint, expected, std::move(on_send));

  ASSERT_EQ(called, 0U);

  io_context_.run();

  ASSERT_EQ(called, 1U);
}
