// SPDX-License-Identifier: MIT

#include <array>
#include <vector>

#include <gtest/gtest.h>

#include <ks/dht/detail/candidate_tracker.hpp>
#include <ks/dht/detail/peer.hpp>

#include "peer_factory.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

TEST(candidate_tracker_test_construction, can_be_constructed_without_candidates)
{
  kd::id const key{};
  kd::candidate_tracker tracker{ key };

  ASSERT_TRUE(std::ranges::empty(tracker.select_new_candidates(1)));
  ASSERT_TRUE(std::ranges::empty(tracker.select_alive_candidates(1)));
  ASSERT_EQ(key, tracker.get_key());
}

TEST(candidate_tracker_test_usage, can_select_all_candidates)
{
  kd::id const key{};
  kd::candidate_tracker tracker{ key };

  std::array const candidates{
    create_peer(kd::id{ "7" }),
    create_peer(kd::id{ "2" }),
    create_peer(kd::id{ "6" }),
    create_peer(kd::id{ "1" }),
  };

  tracker.add_candidates(candidates);

  {
    auto const new_candidates =
        tracker.select_new_candidates(4) | std::ranges::to<std::vector>();
    ASSERT_EQ(4, new_candidates.size());
    ASSERT_EQ(kd::id{ "1" }, new_candidates[0].id);
    ASSERT_EQ(kd::id{ "2" }, new_candidates[1].id);
    ASSERT_EQ(kd::id{ "6" }, new_candidates[2].id);
    ASSERT_EQ(kd::id{ "7" }, new_candidates[3].id);
  }
}

TEST(candidate_tracker_test_usage, can_select_some_candidates)
{
  kd::id const key{};
  kd::candidate_tracker tracker{ key };

  std::array const candidates{
    create_peer(kd::id{ "7" }), create_peer(kd::id{ "3" }),
    create_peer(kd::id{ "6" }), create_peer(kd::id{ "18" }),
    create_peer(kd::id{ "2" }), create_peer(kd::id{ "9" }),
    create_peer(kd::id{ "1" }),
  };

  tracker.add_candidates(candidates);

  {
    auto const new_candidates =
        tracker.select_new_candidates(2) | std::ranges::to<std::vector>();
    ASSERT_EQ(2, new_candidates.size());
    ASSERT_EQ(kd::id{ "1" }, new_candidates[0].id);
    ASSERT_EQ(kd::id{ "2" }, new_candidates[1].id);
  }
}

TEST(candidate_tracker_test_usage, can_ignore_selected_candidates)
{
  kd::id const key{};
  kd::candidate_tracker tracker{ key };

  std::array const candidates{
    create_peer(kd::id{ "7" }), create_peer(kd::id{ "3" }),
    create_peer(kd::id{ "6" }), create_peer(kd::id{ "18" }),
    create_peer(kd::id{ "2" }), create_peer(kd::id{ "9" }),
    create_peer(kd::id{ "1" }),
  };

  tracker.add_candidates(candidates);

  {
    auto const new_candidates =
        tracker.select_new_candidates(2) | std::ranges::to<std::vector>();
    ASSERT_EQ(2, new_candidates.size());
    ASSERT_EQ(kd::id{ "1" }, new_candidates[0].id);
    ASSERT_EQ(kd::id{ "2" }, new_candidates[1].id);
  }

  {
    auto const new_candidates =
        tracker.select_new_candidates(2) | std::ranges::to<std::vector>();
    ASSERT_EQ(2, new_candidates.size());
    ASSERT_EQ(kd::id{ "3" }, new_candidates[0].id);
    ASSERT_EQ(kd::id{ "6" }, new_candidates[1].id);
  }
}

TEST(candidate_tracker_test_usage, can_aliveate_candidates)
{
  kd::id const key{};
  kd::candidate_tracker tracker{ key };

  std::array const candidates{
    create_peer(kd::id{ "2" }),
    create_peer(kd::id{ "1" }),
  };

  tracker.add_candidates(candidates);

  {
    auto const new_candidates =
        tracker.select_new_candidates(2) | std::ranges::to<std::vector>();
    ASSERT_EQ(2, new_candidates.size());
    ASSERT_EQ(kd::id{ "1" }, new_candidates[0].id);
    ASSERT_EQ(kd::id{ "2" }, new_candidates[1].id);
  }

  {
    ASSERT_TRUE(std::ranges::empty(tracker.select_alive_candidates(1)));
    tracker.flag_candidate_as_alive(kd::id{ "1" });
  }
  {
    auto const alive_candidates =
        tracker.select_alive_candidates(1) | std::ranges::to<std::vector>();
    ASSERT_EQ(1, alive_candidates.size());
    ASSERT_EQ(kd::id{ "1" }, alive_candidates[0].id);
  }

  tracker.flag_candidate_as_dead(kd::id{ "2" });

  {
    auto const alive_candidates =
        tracker.select_alive_candidates(1) | std::ranges::to<std::vector>();
    ASSERT_EQ(1, alive_candidates.size());
    ASSERT_EQ(kd::id{ "1" }, alive_candidates[0].id);
  }
}

TEST(candidate_tracker_test_usage, can_ignore_already_known_candidate)
{
  kd::id const key{};
  kd::candidate_tracker tracker{ key };

  tracker.add_candidate(create_peer(kd::id{ "7" }));

  {
    auto const new_candidates =
        tracker.select_new_candidates(20) | std::ranges::to<std::vector>();
    ASSERT_EQ(1, new_candidates.size());
    ASSERT_EQ(kd::id{ "7" }, new_candidates[0].id);
  }

  tracker.add_candidate(create_peer(kd::id{ "7" }));

  ASSERT_TRUE(std::ranges::empty(tracker.select_new_candidates(20)));
}

TEST(candidate_tracker_test_usage, can_accept_iterators_as_candidates)
{
  kd::id const key{};
  kd::candidate_tracker tracker{ key };

  std::array const candidates{
    create_peer(kd::id{ "7" }),
    create_peer(kd::id{ "2" }),
    create_peer(kd::id{ "6" }),
    create_peer(kd::id{ "1" }),
  };

  tracker.add_candidates(candidates.begin(), candidates.end());

  {
    auto const new_candidates =
        tracker.select_new_candidates(4) | std::ranges::to<std::vector>();
    ASSERT_EQ(4, new_candidates.size());
    ASSERT_EQ(kd::id{ "1" }, new_candidates[0].id);
    ASSERT_EQ(kd::id{ "2" }, new_candidates[1].id);
    ASSERT_EQ(kd::id{ "6" }, new_candidates[2].id);
    ASSERT_EQ(kd::id{ "7" }, new_candidates[3].id);
  }
}
