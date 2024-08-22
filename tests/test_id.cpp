// SPDX-License-Identifier: MIT

#include <sstream>
#include <system_error>

#include <gtest/gtest.h>

#include <ks/serialization/as_bytes.hpp>

#include <ks/dht/detail/id.hpp>

#include "common.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

TEST(id_test_construction, random_generated_id_are_different)
{
  std::default_random_engine random_engine;

  ASSERT_NE(kd::id{ random_engine }, kd::id{ random_engine });
}

TEST(id_test_construction, emptry_string_generated_id_is_valid)
{
  kd::id const new_id{ "" };

  ASSERT_EQ(new_id, kd::id{});
}

TEST(id_test_construction, invalid_string_cannot_generate_id)
{
  ASSERT_THROW(kd::id("?"), std::system_error);
  ASSERT_THROW(kd::id("1?2"), std::system_error);
  ASSERT_THROW(kd::id("x"), std::system_error);
  ASSERT_THROW(kd::id(" "), std::system_error);
  ASSERT_THROW(kd::id(std::string(41ULL, '0')), std::system_error);
}

TEST(id_test_construction, valid_string_generates_valid_id)
{
  {
    kd::id const new_id{ "fedcba9876543210" };

    std::size_t i = 0;
    while (i != 96)
      ASSERT_FALSE(new_id[i++]);

    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);

    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);

    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);

    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);

    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);

    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);

    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);

    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);

    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);

    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);

    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);

    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);

    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);

    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);

    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_TRUE(new_id[i++]);

    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
    ASSERT_FALSE(new_id[i++]);
  }
  {
    kd::id const new_id{ "8000"
                         "0000"
                         "0000"
                         "0000"
                         "0000"
                         "0000"
                         "0000"
                         "0000"
                         "0000"
                         "0001" };

    ASSERT_TRUE(new_id[0]);
    ASSERT_FALSE(new_id[1]);
    ASSERT_FALSE(new_id[158]);
    ASSERT_TRUE(new_id[159]);
  }
}

TEST(id_test_construction, hash_generated_id_are_valid)
{
  kd::id const id1{ ks::serialization::as_bytes("value to hash") };
  kd::id const id2{ ks::serialization::as_bytes("other value to hash") };
  ASSERT_NE(id1, id2);
}

TEST(id_test_operation, id_can_be_sorted)
{
  ASSERT_LT(kd::id{ "0" }, kd::id{ "1" });
  ASSERT_LT(kd::id{ "1" }, kd::id{ "2" });
  ASSERT_LT(kd::id{ "1" }, kd::id{ "3" });
  ASSERT_LT(kd::id{ "1" }, kd::id{ "8000000000000" });
}

TEST(id_test_operation, id_bit_can_be_updated)
{
  kd::id i{ "0" };
  i[0] = true;
  i[kd::id::bit_size - 1] = true;
  i[kd::id::bit_size - 2] = true;

  ASSERT_EQ(kd::id{ "8000000000000000000000000000000000000003" }, i);

  i[kd::id::bit_size - 1] = false;

  ASSERT_EQ(kd::id{ "8000000000000000000000000000000000000002" }, i);
}

TEST(id_test_operation, id_distance_can_be_evaluated)
{
  {
    std::default_random_engine random_engine;

    kd::id id{ random_engine };

    ASSERT_EQ(kd::id(), kd::distance(id, id));
  }
  {
    kd::id const id1{ "1" };
    kd::id const id2{ "2" };
    kd::id const id3{ "4" };

    ASSERT_LT(kd::distance(id1, id2), kd::distance(id1, id3));
  }
}

/**
 *  Test operator<<()
 */
TEST(id_test_print, id_is_printable)
{
  kd::id const id{ "0123456789abcdef" };

  ASSERT_EQ(std::format("{}", id), "0123456789abcdef");
}
