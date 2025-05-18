// SPDX-License-Identifier: MIT

#include <random>
#include <cstdint>

#include <gtest/gtest.h>

#include <ks/dht/detail/routing_table.hpp>
#include <ks/dht/endpoint.hpp>

namespace k = ks::dht;
namespace kd = k::detail;

namespace {

constexpr auto subtree_depth{5U};
constexpr auto bucket_size{20U};

using test_routing_table = kd::routing_table<std::uint8_t, bucket_size, subtree_depth>;

} // anonymous namespace

TEST(routing_table_test_construction, is_empty_on_construction)
{
  std::default_random_engine random_engine;

  // Create an empty routing_table.
  test_routing_table rt{ kd::id{ random_engine }};
}

TEST(routing_table_test_construction, can_add_peer)
{
  test_routing_table rt{ kd::id{}};

  rt.push({kd::id{"1"}, 1U});
}

#if 0
TEST(routing_table_test_push, closest_k_bucket_can_receive_unlimited_peers)
{
  // My id is 128 bit assigned to 0.
  kd::id const my_id;
  // Each bucket can contain up to 4 peers.
  std::size_t const bucket_size = 2;

  test_routing_table rt{ my_id, bucket_size };

  // This peer will be associated with every id.
  // Unicity applies only to id, not peer.
  constexpr std::uint8_t test_peer{};

  // Theses should go into close bucket.
  ASSERT_TRUE(rt.push(kd::id{ "10" }, test_peer));
  ASSERT_TRUE(rt.push(kd::id{ "11" }, test_peer));

  // While theses go to a far bucket.
  ASSERT_TRUE(rt.push(kd::id{ "20" }, test_peer));
  ASSERT_TRUE(rt.push(kd::id{ "21" }, test_peer));

  // Only the close bucket can overflow.
  ASSERT_FALSE(rt.push(kd::id{ "22" }, test_peer));

  // This one can be saved as the close bucket
  // is allowed to exceed bucket_size.
  ASSERT_TRUE(rt.push(kd::id{ "12" }, test_peer));
  ASSERT_TRUE(rt.push(kd::id{ "13" }, test_peer));
}

TEST(routing_table_test_push, discards_already_pushed_ids)
{
  std::default_random_engine random_engine;

  test_routing_table rt{ kd::id{ random_engine } };
  constexpr std::uint8_t test_peer{};
  kd::id test_id;

  // Push two times the same peer.
  ASSERT_EQ(rt.push(test_id, test_peer), true);
  // Expect the second call to fail.
  ASSERT_EQ(rt.push(test_id, test_peer), false);
  ASSERT_EQ(rt.peer_count(), 1);
}

TEST(routing_table_test_find, can_find_a_peer)
{
  test_routing_table rt{ kd::id{} };
  constexpr std::uint8_t test_peer{};
  kd::id test_id{ "a" };
  ASSERT_TRUE(rt.push(test_id, test_peer));

  // Try to find the generated peer.
  auto i = rt.find(test_id);
  ASSERT_NE(i, rt.end());
  ASSERT_EQ(test_peer, i->second);
}

TEST(routing_table_test_find, can_find_a_closer_peer)
{
  test_routing_table rt{ kd::id{} };

  constexpr std::uint8_t test_peer1{};
  kd::id test_id1{ "1" };
  ASSERT_TRUE(rt.push(test_id1, test_peer1));

  constexpr std::uint8_t test_peer2{1};
  kd::id test_id2{ "2" };
  ASSERT_TRUE(rt.push(test_id2, test_peer2));

  constexpr std::uint8_t test_peer3{2};
  kd::id test_id3{ "4" };
  ASSERT_TRUE(rt.push(test_id3, test_peer3));

  // test_peer2 is the closest peer.
  auto i = rt.find(kd::id{ "6" });
  ASSERT_NE(i, rt.end());
  ASSERT_EQ(test_peer2, i->second);
}

TEST(routing_table_test_find, iterator_start_from_the_closest_k_bucket)
{
  test_routing_table rt(kd::id{}, 1);
  constexpr std::uint8_t test_peer1{};
  kd::id id1{ "1" };
  ASSERT_TRUE(rt.push(id1, test_peer1));

  constexpr std::uint8_t test_peer2{1};
  kd::id id2{ "2" };
  ASSERT_TRUE(rt.push(id2, test_peer2));

  constexpr std::uint8_t test_peer3{2};
  kd::id id3{ "4" };
  ASSERT_TRUE(rt.push(id3, test_peer3));

  // Ask for id of the closest peer, and expect to see
  // all of them.
  auto i = rt.find(id1);

  // This one should be in the close bucket.
  ASSERT_NE(i, rt.end());
  ASSERT_EQ(test_peer1, i->second);
  ++i;

  // This one too.
  ASSERT_NE(i, rt.end());
  ASSERT_EQ(test_peer2, i->second);
  ++i;

  // This one in the far bucket.
  ASSERT_NE(i, rt.end());
  ASSERT_EQ(test_peer3, i->second);
  ++i;

  ASSERT_EQ(i, rt.end());
}

TEST(routing_table_test_find, iterator_skip_empty_k_bucket)
{
  test_routing_table rt{ kd::id{}, 1 };
  // Fill far k_bucket.
  constexpr std::uint8_t test_peer1{};
  kd::id id1{ "1" };
  ASSERT_TRUE(rt.push(id1, test_peer1));

  // Skip the next "2".

  // End with this one.
  constexpr std::uint8_t test_peer2{1};
  kd::id id2{ "4" };
  ASSERT_TRUE(rt.push(id2, test_peer2));

  // Ask for id of the closest peer, and expect to see
  // all of them.
  auto i = rt.find(id1);

  ASSERT_NE(i, rt.end());
  ASSERT_EQ(test_peer1, i->second);
  ++i;

  ASSERT_NE(i, rt.end());
  ASSERT_EQ(test_peer2, i->second);
  ++i;

  ASSERT_EQ(i, rt.end());
}

TEST(routing_table_test_remove, can_remove_a_peer)
{
  test_routing_table rt{ kd::id{} };
  constexpr std::uint8_t test_peer{};
  kd::id test_id{};
  ASSERT_TRUE(rt.push(test_id, test_peer));

  // Try to find the generated peer.
  ASSERT_TRUE(rt.find(test_id) != rt.end());
  std::size_t saved_table_size = rt.peer_count();
  ASSERT_EQ(rt.remove(test_id), true);
  ASSERT_EQ(rt.peer_count(), saved_table_size - 1);
  ASSERT_TRUE(rt.find(test_id) == rt.end());
}

TEST(routing_table_test_print, print_empty_test_routing_table)
{
  auto const out = std::format("{}", test_routing_table{ kd::id{ "a" }, 20 });
  ASSERT_EQ(out, R"({"id":"0a","peer_count":0,"k_bucket_size":20})");
}

#endif
