// SPDX-License-Identifier: MIT

#include <chrono>
#include <memory>

#include <gtest/gtest.h>

#include <asio/as_tuple.hpp>
#include <asio/awaitable.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/cancellation_signal.hpp>
#include <asio/co_spawn.hpp>
#include <asio/compose.hpp>
#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>

#include <ks/dht/detail/async_queue.hpp>
#include <ks/dht/detail/timer.hpp>

#include "fixture.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

using namespace std::literals;

class test_async_queue : public io_fixture
{
protected:
  auto init_task(std::chrono::nanoseconds duration)
  {
    return [this, duration](auto on_complete) {
      ++scheduled_tasks_;

      EXPECT_FALSE(task_running_);
      task_running_ = true;

      auto timer =
          std::make_shared<kd::timer<executor_type>>(executor_, duration);

      auto on_timeout = [this, on_complete = std::move(on_complete), timer](
                            std::error_code failure) mutable {
        task_running_ = false;
        std::move(on_complete)(failure);
      };

      auto slot = asio::get_associated_cancellation_slot(on_complete);
      timer->async_wait(
          asio::bind_cancellation_slot(slot, std::move(on_timeout)));
    };
  }

  auto expect_success()
  {
    return [this](std::error_code failure) {
      ++terminated_tasks_;
      EXPECT_FALSE(failure);
    };
  }

  auto expect_failure()
  {
    return [this](std::error_code failure) {
      ++terminated_tasks_;
      EXPECT_TRUE(failure);
    };
  };

protected:
  std::size_t scheduled_tasks_{};
  std::size_t terminated_tasks_{};
  bool task_running_{};
  kd::async_queue queue_{};
  asio::cancellation_signal signal_{};
};

TEST_F(test_async_queue, can_queue_message)
{
  queue_.queue(init_task(1ns), expect_success());
  queue_.queue(init_task(1ns), expect_success());
  queue_.queue(init_task(1ns), expect_success());

  io_context_.run();

  ASSERT_EQ(3U, scheduled_tasks_);
  ASSERT_EQ(3U, terminated_tasks_);
}

TEST_F(test_async_queue, can_cancel_spawned_coroutines)
{
  queue_.queue(init_task(10s),
               asio::bind_cancellation_slot(signal_.slot(), expect_failure()));

  io_context_.poll();

  signal_.emit(asio::cancellation_type::total);

  io_context_.run();

  EXPECT_EQ(terminated_tasks_, 1U);
}

TEST_F(test_async_queue, can_cancel_queued_coroutines)
{
  queue_.queue(init_task(1ns), expect_success());

  queue_.queue(init_task(10s),
               asio::bind_cancellation_slot(signal_.slot(), expect_failure()));
  signal_.emit(asio::cancellation_type::total);

  queue_.queue(init_task(1ns), expect_success());

  io_context_.run();

  EXPECT_EQ(terminated_tasks_, 3U);
}
