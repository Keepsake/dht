// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <exception>
#include <string>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>

#include <ks/dht/detail/rethrow_exception.hpp>
#include <ks/dht/detail/socket.hpp>
#include <ks/dht/detail/use_awaitable.hpp>
#include <ks/dht/endpoint.hpp>

#include "common.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

class test_socket : public testing::Test
{
protected:
  asio::io_context io_context_{};
};

TEST_F(test_socket, can_socket_v4_loopback)
{
  auto test = [&] -> asio::awaitable<void, asio::io_context::executor_type> {
    kd::socket sender{ io_context_.get_executor(),
                       k::endpoint_v4{ asio::ip::make_address_v4("127.0.0.1"),
                                       0 } };

    kd::socket receiver{ io_context_.get_executor(),
                         k::endpoint_v4{ asio::ip::make_address_v4("127.0.0.1"),
                                         0 } };

    std::string const expected("data");
    co_await sender.async_send_to(receiver.get_local_endpoint(), expected);

    k::endpoint_v4 sender_endpoint;
    std::string actual;
    co_await receiver.async_receive_from(sender_endpoint, actual);

    EXPECT_EQ(expected, actual);
    EXPECT_EQ(sender_endpoint, sender.get_local_endpoint());
  };

  co_spawn(io_context_.get_executor(), test, kd::rethrow_exception);
  io_context_.run();
}

TEST_F(test_socket, can_socket_v6_loopback)
{
  auto test = [&] -> asio::awaitable<void, asio::io_context::executor_type> {
    kd::socket sender{ io_context_.get_executor(),
                       k::endpoint_v6{ asio::ip::make_address_v6("::1"), 0 } };

    kd::socket receiver{ io_context_.get_executor(),
                         k::endpoint_v6{ asio::ip::make_address_v6("::1"),
                                         0 } };

    std::string const expected("data");
    co_await sender.async_send_to(receiver.get_local_endpoint(), expected);

    k::endpoint_v6 sender_endpoint;
    std::string actual;
    co_await receiver.async_receive_from(sender_endpoint, actual);

    EXPECT_EQ(expected, actual);
    EXPECT_EQ(sender_endpoint, sender.get_local_endpoint());
  };

  co_spawn(io_context_.get_executor(), test, kd::rethrow_exception);
  io_context_.run();
}

TEST_F(test_socket, can_queue_message)
{
  kd::socket sender{ io_context_.get_executor(),
                     k::endpoint_v6{ asio::ip::make_address_v6("::1"), 0 } };

  kd::socket receiver{ io_context_.get_executor(),
                       k::endpoint_v6{ asio::ip::make_address_v6("::1"), 0 } };

  std::string const expected("data");
  auto send_data =
      [&] -> asio::awaitable<void, asio::io_context::executor_type> {
    co_await sender.async_send_to(receiver.get_local_endpoint(), expected);
  };

  auto receive_data =
      [&] -> asio::awaitable<void, asio::io_context::executor_type> {
    k::endpoint_v6 sender_endpoint;
    std::string actual;
    for (std::size_t i = 0; i != 3; ++i) {
      co_await receiver.async_receive_from(sender_endpoint, actual);

      EXPECT_EQ(expected, actual);
      EXPECT_EQ(sender_endpoint, sender.get_local_endpoint());
    }
  };

  for (std::size_t i = 0; i != 3; ++i)
    co_spawn(io_context_.get_executor(), send_data, kd::rethrow_exception);

  co_spawn(io_context_.get_executor(), receive_data, kd::rethrow_exception);

  io_context_.run();
}
