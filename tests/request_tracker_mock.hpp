// SPDX-License-Identifier: MIT

#pragma once

#include <system_error>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <asio/any_completion_handler.hpp>

#include <ks/dht/detail/message.hpp>
#include <ks/dht/endpoint.hpp>

struct request_tracker_mock final
{
  using on_reply = asio::any_completion_handler<void(std::error_code)>;

  MOCK_METHOD(void,
              send_request,
              (ks::dht::endpoint const& endpoint,
               ks::dht::detail::message_body const& request_body,
               ks::dht::detail::message_body& response_body,
               on_reply on_reply),
              ());
};
