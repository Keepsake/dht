// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ks/dht/detail/id.hpp>
#include <ks/dht/endpoint.hpp>

struct routing_table_mock final
{
  MOCK_METHOD(void,
              push,
              (ks::dht::detail::id const& id,
               ks::dht::endpoint const& endpoint));
};
