// SPDX-License-Identifier: MIT

#include <format>
#include <random>
#include <system_error>

#include <gtest/gtest.h>

#include <ks/serialization/as_bytes.hpp>
#include <ks/serialization/serialize.hpp>

#include <ks/dht/detail/id.hpp>

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
  ASSERT_THROW(kd::id(std::string(65ULL, '0')), std::system_error);
}

TEST(id_test_construction, valid_string_generates_valid_id)
{
  {
    kd::id const new_id{ "5000000000000000"
                         "0000000000000000"
                         "0000000000000000"
                         "000000000000000a" };

    // Check MSB
    ASSERT_FALSE(new_id[0]);
    ASSERT_TRUE(new_id[1]);
    ASSERT_FALSE(new_id[2]);
    ASSERT_TRUE(new_id[3]);

    for (std::size_t i{4U}; i != kd::id::bit_size - 5; ++i)
      ASSERT_FALSE(new_id[i]);

    // Check LSB
    ASSERT_TRUE(new_id[kd::id::bit_size - 4]);
    ASSERT_FALSE(new_id[kd::id::bit_size - 3]);
    ASSERT_TRUE(new_id[kd::id::bit_size - 2]);
    ASSERT_FALSE(new_id[kd::id::bit_size - 1]);
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

TEST(id_test_operation, id_distance_can_be_evaluated)
{
  {
    std::default_random_engine random_engine;

    kd::id id{ random_engine };

    ASSERT_EQ(kd::id(), id ^ id);
  }
  {
    kd::id const id1{ "1" };
    kd::id const id2{ "2" };
    kd::id const id3{ "4" };

    ASSERT_LT(id1 ^ id2, id1 ^ id3);
  }
}

TEST(id_test_iteration, iterator_can_be_derefenced)
{
  kd::id const id{ "1" };

  ASSERT_TRUE(*id.rbegin());
}

TEST(id_test_iteration, iterator_can_be_compared)
{
  kd::id const id{ "1" };

  ASSERT_EQ(id.begin(), id.begin());
  ASSERT_EQ(id.end(), id.end());
  ASSERT_NE(id.begin(), id.end());
}

TEST(id_test_iteration, iterator_can_be_pre_incremented)
{
  kd::id const id{ "1" };

  auto i = id.begin();
  for (std::size_t count{}; count != kd::id::bit_size - 1U; ++count) {
    ASSERT_FALSE(*i);
    ++i;
  }

  ASSERT_TRUE(*i);
  ++i;
  ASSERT_EQ(i, id.end());
}

TEST(id_test_iteration, iterator_can_be_post_incremented)
{
  kd::id const id{ "1" };

  auto i = id.begin();
  for (std::size_t count{}; count != kd::id::bit_size - 1U; ++count)
    ASSERT_FALSE(*i++);

  ASSERT_TRUE(*i++);
  ASSERT_EQ(i, id.end());
}

TEST(id_test_iteration, iterator_can_be_range_incremented)
{
  kd::id const id{ "1" };

  ASSERT_EQ(id.begin() + kd::id::bit_size, id.end());

  auto i = id.begin();
  i += kd::id::bit_size;
  ASSERT_EQ(i, id.end());
}

TEST(id_test_iteration, iterator_can_be_pre_decremented)
{
  kd::id const id{ "1" };

  auto i = id.end();

  ASSERT_TRUE(*--i);

  for (std::size_t count{}; count != kd::id::bit_size - 1U; ++count) {
    ASSERT_FALSE(*--i);
  }

  ASSERT_EQ(i, id.begin());
}

TEST(id_test_iteration, iterator_can_be_post_decremented)
{
  kd::id const id{ "1" };

  auto i = --id.end();
  ASSERT_TRUE(*i--);

  for (std::size_t count{}; count != kd::id::bit_size - 2U; ++count) {
    ASSERT_FALSE(*i--);
  }

  ASSERT_FALSE(*i);
  ASSERT_EQ(i, id.begin());
}

TEST(id_test_iteration, iterator_can_be_range_decremented)
{
  kd::id const id{ "8000"
                   "0000"
                   "0000"
                   "0000"
                   "0000"
                   "0000"
                   "0000"
                   "0000"
                   "0000"
                   "0000" };

  ASSERT_EQ(id.end() - kd::id::bit_size, id.begin());

  auto i = id.end();
  i -= kd::id::bit_size;
  ASSERT_EQ(i, id.begin());
}

TEST(id_test_iteration, iterator_can_be_substracted)
{
  kd::id const id{ "1" };

  ASSERT_EQ(id.end() - id.begin(), kd::id::bit_size);
}

TEST(id_test_iteration, iterator_can_be_subscribed)
{
  kd::id const id{ "2" };

  auto i = id.begin();
  ASSERT_FALSE(i[kd::id::bit_size - 1U]);
  ASSERT_TRUE(i[kd::id::bit_size - 2U]); 
  ASSERT_FALSE(i[kd::id::bit_size - 3U]);
}

TEST(id_test_print, id_is_printable)
{
  kd::id const id{ "0123456789abcdef" };

  ASSERT_EQ(std::format("{}", id), "0123456789abcdef");
}

TEST(id_test_hash, id_can_be_hashed)
{
  std::default_random_engine random_engine;

  kd::id const id1{ random_engine };
  kd::id const id2{ random_engine };

  std::hash<kd::id> hasher{};

  ASSERT_NE(hasher(id1), hasher(id2));

}

TEST(id_test_serialization, id_can_serialized)
{
  std::default_random_engine random_engine;

  std::vector<std::byte> buffer;

  kd::id const expected{ random_engine };
  auto failure = ks::serialization::save(buffer, expected);
  ASSERT_FALSE(failure);

  kd::id actual{};
  failure = ks::serialization::load(buffer, actual);
  ASSERT_FALSE(failure);

  ASSERT_EQ(actual, expected);
}

TEST(id_bit_count, id_can_countl_zero)
{
  {
    kd::id const id{ "1" };
    ASSERT_EQ(id.countl_zero(), kd::id::bit_size - 1U);
  }
  {
    kd::id const id{ "2" };
    ASSERT_EQ(id.countl_zero(), kd::id::bit_size - 2U);
  }
  {
    kd::id const id{ "8000000000000000"
                     "0000000000000000"
                     "0000000000000000"
                     "0000000000000000" };
    ASSERT_EQ(id.countl_zero(), 0U);
  }
}
