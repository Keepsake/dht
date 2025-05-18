// SPDX-License-Identifier: MIT

#include <cstddef>
#include <utility>

#include <gtest/gtest.h>

#include <ks/dht/detail/scope_exit.hpp>

namespace k = ks::dht;
namespace kd = k::detail;

TEST(test_scope_exit, can_be_constructed)
{
  bool called{};
  {
    kd::scope_exit scope{[&called] { called = true; }};
  }

  ASSERT_TRUE(called);
}

TEST(test_scope_exit, can_be_moved)
{
  std::size_t called{};

  {
    kd::scope_exit scope1{[&called] { ++called; }};
    kd::scope_exit scope2{std::move(scope1)};
  }

  ASSERT_EQ(1U, called);
}
