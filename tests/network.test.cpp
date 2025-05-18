// SPDX-License-Identifier: MIT

#include <string>
#include <utility>

#include <gtest/gtest.h>

#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>

#include <ks/dht/endpoint.hpp>

#include <ks/dht/detail/network.hpp>
#include <ks/dht/detail/rethrow_exception.hpp>

#include "fixture.hpp"
#include "socket_mock.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

using testing::A;
using testing::Invoke;

class test_network : public io_fixture
{
protected:
  using socket_v4 = socket_mock<k::endpoint_v4, std::string>;
  using socket_v6 = socket_mock<k::endpoint_v6, std::string>;

protected:
  auto expect_success()
  {
    return [this](std::error_code failure) {
      ++successes_;
      EXPECT_FALSE(failure);
    };
  }

protected:
  socket_v4 socket_v4_{};
  socket_v6 socket_v6_{};
  std::size_t successes_{};
};

TEST_F(test_network, can_send_v4)
{
  kd::network network{ socket_v4_, socket_v6_ };

  std::string const request{ "request v4" };
  k::endpoint_v4 peer_v4{ asio::ip::make_address_v4("127.0.0.2"), 2 };
  k::endpoint peer{ peer_v4 };

  EXPECT_CALL(socket_v4_,
              async_send_to(peer_v4, request, A<socket_v4::on_complete>()))
      .WillOnce(
          Invoke([&](auto const& endpoint, auto const& message, auto on_send) {
            asio::post(executor_,
                       std::bind_front(std::move(on_send), std::error_code{}));
          }));

  network.async_send_to(peer, request, expect_success());

  io_context_.run();

  ASSERT_EQ(successes_, 1U);
}

TEST_F(test_network, can_send_v6)
{
  kd::network network{ socket_v4_, socket_v6_ };

  std::string const request_v6{ "request v6" };
  k::endpoint_v6 peer_v6{ asio::ip::make_address_v6("::2"), 2 };
  k::endpoint peer{ peer_v6 };

  EXPECT_CALL(socket_v6_,
              async_send_to(peer_v6, request_v6, A<socket_v6::on_complete>()))
      .WillOnce(
          Invoke([&](auto const& endpoint, auto const& message, auto on_send) {
            asio::post(executor_,
                       std::bind_front(std::move(on_send), std::error_code{}));
          }));

  network.async_send_to(peer, request_v6, expect_success());

  io_context_.run();

  ASSERT_EQ(successes_, 1U);
}

TEST_F(test_network, can_receive_v4)
{
  kd::network network{ socket_v4_, socket_v6_ };

  k::endpoint_v4 actual_endpoint{};
  k::endpoint_v4 expected_endpoint{ asio::ip::make_address_v4("127.0.0.2"), 2 };
  std::string actual_message;
  std::string const expected_message{ "response v4" };

  EXPECT_CALL(socket_v4_,
              async_receive_from(actual_endpoint, actual_message, A<socket_v4::on_complete>()))
      .WillOnce(
          Invoke([&](auto & endpoint, auto & message, auto on_receive) {
            endpoint = expected_endpoint;
            message = expected_message;
            asio::post(executor_,
                       std::bind_front(std::move(on_receive), std::error_code{}));
          }));

  network.async_receive_from(actual_endpoint, actual_message, expect_success());

  io_context_.run();

  ASSERT_EQ(successes_, 1U);
  ASSERT_EQ(actual_endpoint, expected_endpoint);
  ASSERT_EQ(actual_message, expected_message);
}

TEST_F(test_network, can_receive_v6)
{
  kd::network network{ socket_v4_, socket_v6_ };

  k::endpoint_v6 actual_endpoint{};
  k::endpoint_v6 expected_endpoint{ asio::ip::make_address_v6("::2"), 2 };
  std::string actual_message;
  std::string const expected_message{ "response v6" };

  EXPECT_CALL(socket_v6_,
              async_receive_from(actual_endpoint, actual_message, A<socket_v6::on_complete>()))
      .WillOnce(
          Invoke([&](auto & endpoint, auto & message, auto on_receive) {
            endpoint = expected_endpoint;
            message = expected_message;
            asio::post(executor_,
                       std::bind_front(std::move(on_receive), std::error_code{}));
          }));

  network.async_receive_from(actual_endpoint, actual_message, expect_success());

  io_context_.run();

  ASSERT_EQ(successes_, 1U);
  ASSERT_EQ(actual_endpoint, expected_endpoint);
  ASSERT_EQ(actual_message, expected_message);
}

TEST_F(test_network, can_close)
{
  kd::network network{ socket_v4_, socket_v6_ };

  EXPECT_CALL(socket_v6_, close());
  EXPECT_CALL(socket_v4_, close());

  network.stop();
}
