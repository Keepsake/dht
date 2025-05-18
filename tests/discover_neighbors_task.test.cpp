// SPDX-License-Identifier: MIT

#include <functional>
#include <queue>
#include <random>
#include <system_error>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <asio/post.hpp>

#include <ks/dht/detail/discover_neighbors_task.hpp>

#include "fixture.hpp"
#include "request_tracker_mock.hpp"
#include "routing_table_mock.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

using ::testing::A;
using ::testing::Invoke;

class test_discover_neighbors_task : public io_fixture
{
protected:
  auto reply_timeout()
  {
    return [this](auto const& endpoint,
                  auto const& request,
                  auto& response,
                  auto on_reply) {
      asio::post(executor_,
                 std::bind_front(std::move(on_reply),
                                 make_error_code(std::errc::timed_out)));
    };
  }

  auto reply_response(kd::message_body expected_response)
  {
    return [expected_response, this](auto const& endpoint,
                                     auto const& request,
                                     auto& response,
                                     auto on_reply) {
      response = expected_response;
      asio::post(executor_,
                 std::bind_front(std::move(on_reply), std::error_code{}));
    };
  }

  auto expect_success()
  {
    return [this](std::error_code failure) {
      ASSERT_FALSE(failure);
      is_complete_ = true;
    };
  }

  auto expect_failure(std::error_code expect_failure)
  {
    return [this, expect_failure](std::error_code failure) {
      ASSERT_EQ(failure, expect_failure);
      is_complete_ = true;
    };
  }

protected:
  std::default_random_engine random_engine_{};
  routing_table_mock routing_table_{};
  request_tracker_mock request_tracker_{};
  kd::id my_id{ random_engine_ };
  kd::message_body request_{ kd::find_peer_request_body{ my_id } };
  kd::peer peer1_{
    .id = kd::id{ "1" },
    .endpoint =
        k::endpoint_v4{
            .ip = asio::ip::make_address_v4("192.168.1.1"),
            .port = 1,
        },
  };
  kd::peer peer2_{
    .id = kd::id{ "2" },
    .endpoint =
        k::endpoint_v4{
            .ip = asio::ip::make_address_v4("192.168.1.2"),
            .port = 2,
        },
  };
  kd::peer peer3_{
    .id = kd::id{ "3" },
    .endpoint =
        k::endpoint_v4{
            .ip = asio::ip::make_address_v4("192.168.1.3"),
            .port = 3,
        },
  };
  bool is_complete_{};
};

TEST_F(test_discover_neighbors_task,
       can_notify_error_when_there_no_initial_endpoint)
{
  std::queue<k::endpoint> endpoints;

  kd::async_discover_neighbors(
      my_id,
      request_tracker_,
      routing_table_,
      std::move(endpoints),
      expect_failure(k::error::initial_peer_failed_to_respond));

  io_context_.run();

  EXPECT_TRUE(is_complete_);
}

TEST_F(test_discover_neighbors_task,
       can_notify_error_when_initial_endpoints_fail_to_respond)
{
  std::queue<k::endpoint> endpoints;
  endpoints.push(peer1_.endpoint);
  endpoints.push(peer2_.endpoint);

  EXPECT_CALL(request_tracker_,
              send_request(peer1_.endpoint,
                           request_,
                           A<kd::message_body&>(),
                           A<request_tracker_mock::on_reply>()))
      .WillOnce(Invoke(reply_timeout()));

  EXPECT_CALL(request_tracker_,
              send_request(peer2_.endpoint,
                           request_,
                           A<kd::message_body&>(),
                           A<request_tracker_mock::on_reply>()))
      .WillOnce(Invoke(reply_timeout()));

  kd::async_discover_neighbors(
      my_id,
      request_tracker_,
      routing_table_,
      std::move(endpoints),
      expect_failure(k::error::initial_peer_failed_to_respond));

  io_context_.run();

  EXPECT_TRUE(is_complete_);
}

TEST_F(test_discover_neighbors_task, can_contact_endpoints_until_one_respond)
{
  std::queue<k::endpoint> endpoints;
  endpoints.push(peer1_.endpoint);
  endpoints.push(peer2_.endpoint);

  EXPECT_CALL(request_tracker_,
              send_request(peer1_.endpoint,
                           request_,
                           A<kd::message_body&>(),
                           A<request_tracker_mock::on_reply>()))
      .WillOnce(Invoke(reply_timeout()));

  EXPECT_CALL(request_tracker_,
              send_request(peer2_.endpoint,
                           request_,
                           A<kd::message_body&>(),
                           A<request_tracker_mock::on_reply>()))
      .WillOnce(Invoke(reply_response(
          kd::find_peer_response_body{ std::vector{ peer3_ } })));

  kd::async_discover_neighbors(my_id,
                               request_tracker_,
                               routing_table_,
                               std::move(endpoints),
                               expect_success());

  io_context_.run();

  EXPECT_TRUE(is_complete_);
}

TEST_F(test_discover_neighbors_task, can_skip_wrong_response)
{
  std::queue<k::endpoint> endpoints;
  endpoints.push(peer1_.endpoint);

  EXPECT_CALL(request_tracker_,
              send_request(peer1_.endpoint,
                           request_,
                           A<kd::message_body&>(),
                           A<request_tracker_mock::on_reply>()))
      .WillOnce(Invoke(reply_response(kd::find_value_response_body{})));

  kd::async_discover_neighbors(
      my_id,
      request_tracker_,
      routing_table_,
      std::move(endpoints),
      expect_failure(k::error::initial_peer_failed_to_respond));

  io_context_.run();

  EXPECT_TRUE(is_complete_);
}
