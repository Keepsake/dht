// SPDX-License-Identifier: MIT

#pragma once

#include <system_error>

#include <gmock/gmock.h>

#include <asio/any_completion_handler.hpp>

#include <ks/dht/detail/message.hpp>
#include <ks/dht/endpoint.hpp>

struct network_mock final
{
  using handler = asio::any_completion_handler<void(std::error_code)>;

  MOCK_METHOD(void,
              async_send_to,
              (ks::dht::endpoint const& endpoint,
               ks::dht::detail::const_message_view const& request,
               handler handler),
              ());
};
