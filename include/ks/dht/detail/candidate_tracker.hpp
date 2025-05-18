// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <utility>

#include <ks/dht/detail/id.hpp>
#include <ks/dht/detail/peer.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

class candidate_tracker final
{
public:
  constexpr explicit candidate_tracker(id key)
    : key_{ std::move(key) }
  {
  }

  constexpr void flag_candidate_as_alive(id const& candidate_id)
  {
    if (auto candidate = find_candidate(candidate_id); candidate)
      candidate->state = candidate_state::alive;
  }

  constexpr void flag_candidate_as_dead(id const& candidate_id)
  {
    if (auto candidate = find_candidate(candidate_id); candidate)
      candidate->state = candidate_state::dead;
  }

  constexpr auto select_new_candidates(std::size_t max_count)
  {
    return std::ranges::ref_view{ candidates_ } | std::views::values |
           std::views::filter(is_candidate_new) | std::views::take(max_count) |
           std::views::transform(flag_candidate_as_contacted) |
           std::views::transform(&candidate::peer);
  }

  constexpr auto select_alive_candidates(std::size_t max_count) const
  {
    return std::ranges::ref_view{ candidates_ } | std::views::values |
           std::views::filter(is_candidate_alive) |
           std::views::take(max_count) |
           std::views::transform(&candidate::peer);
  }

  constexpr void add_candidate(peer p)
  {
    candidates_.emplace(
        p.id ^ key_,
        candidate{ .peer = std::move(p), .state = candidate_state::unknown });
  }

  constexpr void add_candidates(auto const& peers)
  {
    for (auto const& p : peers)
      add_candidate(p);
  }

  constexpr void add_candidates(auto begin, auto end)
  {
    add_candidates(std::ranges::subrange{ begin, end });
  }

  constexpr id const& get_key() const noexcept { return key_; }

private:
  enum class candidate_state
  {
    unknown,
    contacted,
    alive,
    dead,
  };

  using candidate_peer = peer;

  struct candidate final
  {
    candidate_peer peer;
    candidate_state state;
  };

  using candidates = std::map<id, candidate>;

private:
  static constexpr bool is_candidate_new(candidate const& candidate) noexcept
  {
    return candidate.state == candidate_state::unknown;
  }

  static constexpr bool is_candidate_alive(candidate const& candidate) noexcept
  {
    return candidate.state == candidate_state::alive;
  }

  static constexpr candidate const& flag_candidate_as_contacted(
      candidate& candidate) noexcept
  {
    candidate.state = candidate_state::contacted;
    return candidate;
  }

  constexpr candidate* find_candidate(id const& candidate_id) noexcept
  {
    auto found = candidates_.find(candidate_id ^ key_);
    if (found == candidates_.end())
      return nullptr;

    return &found->second;
  }

private:
  id key_;
  candidates candidates_{};
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht
