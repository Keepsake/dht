// SPDX-License-Identifier: MIT

#include <chrono>
#include <functional>
#include <random>
#include <system_error>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <asio/associated_cancellation_slot.hpp>
#include <asio/bind_cancellation_slot.hpp>

#include <ks/dht/detail/request_tracker.hpp>

#include "fixture.hpp"
#include "network_mock.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

using testing::A;
using testing::Invoke;

class test_request_tracker : public io_fixture
{
protected:
  auto on_send(auto action)
  {
    return [&, action = std::move(action)](
               auto const& endpoint, auto const& request, auto on_send) {
      EXPECT_EQ(endpoint, expected_endpoint_);
      EXPECT_EQ(request.body, expected_request_);
      EXPECT_EQ(request.header.source_id, my_id_);

      asio::post(
          executor_,
          std::bind_front(std::move(action), request, std::move(on_send)));
    };
  }

  auto expect_success()
  {
    return [this](std::error_code failure) {
      EXPECT_FALSE(failure);
      is_complete_ = true;
    };
  }

  auto expect_failure(std::error_code expected_failure)
  {
    return [this, expected_failure](std::error_code failure) {
      EXPECT_EQ(failure, expected_failure);
      is_complete_ = true;
    };
  }

protected:
  std::default_random_engine random_engine_{};
  kd::id my_id_{ random_engine_ };
  network_mock network_{};
  k::endpoint expected_endpoint_{
    k::endpoint_v4{ asio::ip::make_address_v4("127.0.0.2"), 2 }
  };
  kd::message_body expected_request_{ kd::find_value_request_body{
      kd::id{ random_engine_ } } };
  kd::message_body response_{};
  kd::message_body expected_response_{ kd::find_value_response_body{
      .data = { std::byte{ 1 }, std::byte{ 2 }, std::byte{ 3 } },
  } };
  bool is_complete_{};
};

TEST_F(test_request_tracker, can_track_request)
{
  std::chrono::seconds const request_timeout{ 10U };
  kd::request_tracker tracker{
    executor_, my_id_, random_engine_, network_, request_timeout
  };

  auto reply_with_expected_response = [&](auto request, auto on_send) {
    std::move(on_send)(std::error_code{});

    asio::post(executor_, [&, request] {
      tracker.handle_response(request.header.random_token, expected_response_);
    });
  };

  EXPECT_CALL(network_,
              async_send_to(expected_endpoint_,
                            A<kd::const_message_view const&>(),
                            A<network_mock::handler>()))
      .WillOnce(Invoke(on_send(reply_with_expected_response)));

  tracker.send_request(
      expected_endpoint_, expected_request_, response_, expect_success());

  io_context_.run();

  ASSERT_TRUE(is_complete_);
  ASSERT_EQ(response_, expected_response_);
}

TEST_F(test_request_tracker, can_detect_no_reply)
{
  std::chrono::milliseconds const request_timeout{ 1U };
  kd::request_tracker tracker{
    executor_, my_id_, random_engine_, network_, request_timeout
  };

  auto do_not_respond = [&](auto, auto on_send) {
    std::move(on_send)(std::error_code{});
  };

  EXPECT_CALL(network_,
              async_send_to(expected_endpoint_,
                            A<kd::const_message_view const&>(),
                            A<network_mock::handler>()))
      .WillOnce(Invoke(on_send(do_not_respond)));

  tracker.send_request(expected_endpoint_,
                       expected_request_,
                       response_,
                       expect_failure(make_error_code(std::errc::timed_out)));

  io_context_.run();

  ASSERT_TRUE(is_complete_);
  ASSERT_NE(response_, expected_response_);
}

TEST_F(test_request_tracker, can_detect_faulty_send)
{
  std::chrono::seconds const request_timeout{ 10U };
  kd::request_tracker tracker{
    executor_, my_id_, random_engine_, network_, request_timeout
  };

  auto fail_with_error = [&](auto, auto on_send) {
    std::move(on_send)(make_error_code(std::errc::invalid_argument));
  };

  EXPECT_CALL(network_,
              async_send_to(expected_endpoint_,
                            A<kd::const_message_view const&>(),
                            A<network_mock::handler>()))
      .WillOnce(Invoke(on_send(fail_with_error)));

  tracker.send_request(
      expected_endpoint_,
      expected_request_,
      response_,
      expect_failure(make_error_code(std::errc::invalid_argument)));

  io_context_.run();

  ASSERT_TRUE(is_complete_);
  ASSERT_NE(response_, expected_response_);
}

TEST_F(test_request_tracker, can_cancel_send)
{
  std::chrono::seconds const request_timeout{ 10U };
  kd::request_tracker tracker{
    executor_, my_id_, random_engine_, network_, request_timeout
  };

  auto wait_for_cancellation = [this](auto, auto on_send) {
    auto slot = asio::get_associated_cancellation_slot(on_send);
    EXPECT_TRUE(slot.is_connected());
    slot.assign(
        [this, on_send = std::move(on_send)](auto cancellation) mutable {
          EXPECT_EQ(cancellation, asio::cancellation_type::partial);
          asio::post(
              executor_,
              std::bind_front(std::move(on_send),
                              make_error_code(std::errc::operation_canceled)));
        });
  };

  EXPECT_CALL(network_,
              async_send_to(expected_endpoint_,
                            A<kd::const_message_view const&>(),
                            A<network_mock::handler>()))
      .WillOnce(Invoke(on_send(wait_for_cancellation)));

  asio::cancellation_signal signal{};

  tracker.send_request(
      expected_endpoint_,
      expected_request_,
      response_,
      asio::bind_cancellation_slot(
          signal.slot(),
          expect_failure(make_error_code(std::errc::operation_canceled))));

  asio::post(executor_, [&] { signal.emit(asio::cancellation_type::partial); });

  io_context_.run();

  ASSERT_TRUE(is_complete_);
  ASSERT_NE(response_, expected_response_);
}

TEST_F(test_request_tracker, can_cancel_timeout)
{
  std::chrono::seconds const request_timeout{ 10U };
  kd::request_tracker tracker{
    executor_, my_id_, random_engine_, network_, request_timeout
  };

  auto do_not_respond = [&](auto, auto on_send) {
    std::move(on_send)(std::error_code{});
  };

  EXPECT_CALL(network_,
              async_send_to(expected_endpoint_,
                            A<kd::const_message_view const&>(),
                            A<network_mock::handler>()))
      .WillOnce(Invoke(on_send(do_not_respond)));

  asio::cancellation_signal signal{};

  tracker.send_request(
      expected_endpoint_,
      expected_request_,
      response_,
      asio::bind_cancellation_slot(
          signal.slot(),
          expect_failure(make_error_code(std::errc::operation_canceled))));

  asio::post(executor_, [&] { signal.emit(asio::cancellation_type::partial); });

  io_context_.run();

  ASSERT_TRUE(is_complete_);
  ASSERT_NE(response_, expected_response_);
}
