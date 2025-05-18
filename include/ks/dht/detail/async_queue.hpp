// SPDX-License-Identifier: MIT

#pragma once

#include <cassert>
#include <utility>

#include <asio/associated_cancellation_slot.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/cancellation_state.hpp>

#include <ks/dht/detail/completion_wrapper.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

class async_queue final
{
public:
  template<typename Init, typename OnComplete>
  void queue(Init init, OnComplete on_complete)
  {
    queue(std::make_unique<op<Init, OnComplete>>(std::move(init),
                                                 std::move(on_complete)));
  }

private:
  struct op_base
  {
    virtual ~op_base() = default;

    virtual void start(async_queue* queue) = 0;

    bool is_cancelled{};
    std::unique_ptr<op_base> next{};
  };

  template<typename Init, typename OnComplete>
  class op final : public op_base
  {
  public:
    explicit op(Init init, OnComplete on_complete)
      : init_{ std::move(init) }
      , on_complete_{ std::move(on_complete) }
    {
      enable_cancellation();
    }

    virtual void start(async_queue* queue) override
    {
      assert(queue);
      assert(not is_cancelled);

      auto pop_op = [queue](std::error_code failure, auto on_complete) {
        std::move(on_complete)(failure);
        queue->pop_head_and_start_next();
      };

      cancellation_slot_.clear();

      std::move(init_)(
          wrap_completion(std::move(pop_op), std::move(on_complete_)));
    }

  private:
    void enable_cancellation()
    {
      if (not cancellation_slot_.is_connected())
        return;

      cancellation_slot_.assign([this](asio::cancellation_type_t type) {
        std::move(on_complete_)(make_error_code(std::errc::operation_canceled));
        is_cancelled = true;
      });
    }

  private:
    Init init_;
    OnComplete on_complete_;
    asio::cancellation_slot cancellation_slot_{
      asio::get_associated_cancellation_slot(on_complete_)
    };
  };

private:
  void queue(std::unique_ptr<op_base> new_op)
  {
    if (tail_) {
      assert(not tail_->next);
      tail_->next = std::move(new_op);
      tail_ = tail_->next.get();
    } else {
      head_ = std::move(new_op);
      tail_ = head_.get();
      head_->start(this);
    }
  }

  void pop_head_and_start_next()
  {
    assert(head_);
    head_ = std::move(head_->next);

    while (head_ and head_->is_cancelled)
      head_ = std::move(head_->next);

    if (head_)
      head_->start(this);
    else
      tail_ = nullptr;
  }

private:
  std::unique_ptr<op_base> head_{};
  op_base* tail_{};
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
