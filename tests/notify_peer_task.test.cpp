#include <functional>
#include <system_error>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <asio/post.hpp>

#include <ks/dht/detail/notify_peer_task.hpp>

#include "candidate_tracker_mock.hpp"
#include "fixture.hpp"
#include "request_tracker_mock.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

using ::testing::A;
using ::testing::Ge;
using ::testing::Invoke;
using ::testing::Return;

class test_notify_peer_task : public io_fixture
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

protected:
  candidate_tracker_mock candidates_{};
  request_tracker_mock request_tracker_{};
  kd::peer peer1_{
    .id = kd::id{ "1" },
    .endpoint =
        k::endpoint_v4{
            .ip = {},
            .port = 1,
        },
  };
  kd::peer peer2_{
    .id = kd::id{ "2" },
    .endpoint =
        k::endpoint_v4{
            .ip = {},
            .port = 2,
        },
  };
  kd::peer peer3_{
    .id = kd::id{ "3" },
    .endpoint =
        k::endpoint_v4{
            .ip = {},
            .port = 3,
        },
  };
  kd::message_body const request_{ kd::find_peer_request_body{
      kd::id{ "10" } } };
};

TEST_F(test_notify_peer_task, can_return_when_routing_table_is_empty)
{
  bool is_complete{};
  auto on_notify = [&] { is_complete = true; };

  EXPECT_CALL(candidates_, select_new_candidates(Ge(1)))
      .WillOnce(Return(std::vector<kd::peer>{}));

  kd::async_notify_peer(candidates_, request_tracker_, std::move(on_notify));

  io_context_.run();

  EXPECT_TRUE(is_complete);
}

TEST_F(test_notify_peer_task, can_return_when_no_peer_is_reachable)
{
  EXPECT_CALL(candidates_, select_new_candidates(Ge(1)))
      .WillOnce(Return(std::vector{ peer1_, peer2_ }));

  EXPECT_CALL(candidates_, get_key())
      .Times(2)
      .WillRepeatedly(Return(kd::id{ "10" }));

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

  bool is_complete{};
  auto on_notify = [&] { is_complete = true; };

  kd::async_notify_peer(candidates_, request_tracker_, std::move(on_notify));

  io_context_.run();

  EXPECT_TRUE(is_complete);
}

TEST_F(test_notify_peer_task,
       can_return_when_a_second_wave_of_peer_is_discovered)
{
  kd::find_peer_response_body const peer2_response{
    .peers = { peer3_ },
  };

  EXPECT_CALL(candidates_, select_new_candidates(Ge(1)))
      .Times(2)
      .WillOnce(Return(std::vector{ peer1_, peer2_ }))
      .WillOnce(Return(std::vector{ peer3_ }));

  EXPECT_CALL(candidates_, get_key())
      .Times(3)
      .WillRepeatedly(Return(kd::id{ "10" }));

  EXPECT_CALL(candidates_, add_candidates(peer2_response.peers));

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
      .WillOnce(Invoke(reply_response(peer2_response)));

  EXPECT_CALL(request_tracker_,
              send_request(peer3_.endpoint,
                           request_,
                           A<kd::message_body&>(),
                           A<request_tracker_mock::on_reply>()))
      .WillOnce(Invoke(reply_timeout()));

  bool is_complete{};
  auto on_receive = [&] { is_complete = true; };

  kd::async_notify_peer(candidates_, request_tracker_, std::move(on_receive));

  io_context_.run();

  EXPECT_TRUE(is_complete);
}
