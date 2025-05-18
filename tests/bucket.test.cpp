// SPDX-License-Identifier: MIT

#include <array>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ks/dht/detail/bucket.hpp>

namespace k = ks::dht;
namespace kd = k::detail;

using testing::ElementsAreArray;

TEST(bucket_test_usage, can_be_default_constructed)
{
  kd::bucket<int, 2> b{};
  EXPECT_EQ(0U, b.size());
}

TEST(bucket_test_usage, can_be_populated)
{
  kd::bucket<int, 2> b{};

  auto ptr = b.try_push_back(1);
  ASSERT_TRUE(ptr);
  EXPECT_EQ(*ptr, 1);
  ptr = b.data();
  ASSERT_TRUE(ptr);
  EXPECT_EQ(*ptr, 1);
  EXPECT_EQ(1U, b.size());

  ptr = b.try_push_back(2);
  ASSERT_TRUE(ptr);
  EXPECT_EQ(*ptr, 2);
  EXPECT_EQ(2U, b.size());

  ptr = b.try_push_back(3);
  ASSERT_FALSE(ptr);
  EXPECT_EQ(2U, b.size());

  auto post_removed = b.erase(b.begin());
  EXPECT_NE(post_removed, b.end());
  EXPECT_EQ(*post_removed, 2);
  EXPECT_EQ(1U, b.size());

  ptr = b.try_push_back(4);
  ASSERT_TRUE(ptr);
  EXPECT_EQ(*ptr, 4);
  EXPECT_EQ(2U, b.size());

  EXPECT_THAT(b, ElementsAreArray({2, 4}));
}
