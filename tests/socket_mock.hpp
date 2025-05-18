// SPDX-License-Identifier: MIT

#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <asio/any_completion_handler.hpp>

#include <ks/dht/detail/message.hpp>

template<typename Endpoint, typename Message>
struct socket_mock final
{
  using endpoint_type = Endpoint;
  using message_type = Message;

  using on_complete = asio::any_completion_handler<void(std::error_code)>;

  MOCK_METHOD(void,
              async_send_to,
              (endpoint_type const& endpoint,
               message_type const& message,
               on_complete on_complete),
              ());

  MOCK_METHOD(void,
              async_receive_from,
              (endpoint_type & endpoint,
               message_type& message,
               on_complete on_complete),
              ());

  MOCK_METHOD(void, close, (), ());
};
