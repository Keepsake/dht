// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <format>
#include <memory>
#include <ranges>
#include <utility>

#include <ks/dht/detail/bucket.hpp>
#include <ks/dht/detail/config.hpp>
#include <ks/dht/detail/id.hpp>

namespace ks::dht::inline abiv1::detail {

template<typename PeerType, std::size_t BucketSize, std::size_t SubtreeDepth>
class routing_table final
{
public:
  using peer_type = PeerType;

  using value_type = std::pair<id, peer_type>;

public:
  constexpr routing_table(id my_id)
    : my_id_{ my_id }
  {
  }

  routing_table(routing_table const&) = delete;

  routing_table& operator=(routing_table const&) = delete;

  bool push(value_type value)
  {
    auto const peer_distance = distance(value.first);

    auto [subtree_index, bucket_index] = find_bucket(peer_distance);

    return (*subtrees_)[subtree_index][bucket_index].try_push_back(
        std::move(value));
  }

  bool remove(id const& peer_id)
  {
    auto const peer_distance = distance(peer_id);

    auto [subtree_index, bucket_index] = find_bucket(peer_distance);

    auto& bucket = (*subtrees_)[subtree_index][bucket_index];

    auto where = std::ranges::find(bucket, peer_id, &value_type::first);
    if (where == bucket.end()) [[unlikely]]
      return false;

    bucket.erase(where);

    return true;
  }

  std::size_t find(id const& id, std::ranges::output_range<peer_type> auto& peers)
  {
    auto const peer_distance = distance(id);

    auto [subtree_index, bucket_index] = find_bucket(peer_distance);

    auto& bucket = (*subtrees_)[subtree_index][bucket_index];
    auto [in, out] =
        std::ranges::copy(bucket | std::ranges::views::take(peers.size()),
                          std::ranges::begin(peers));

    auto peer_found = std::distance(bucket.begin(), in);
    while (peer_found != peers.size()) {
      if (bucket_index) {
      }
    }
    return peer_found;
  }

private:
  using bucket_type = bucket<value_type, BucketSize>;

  constexpr static std::size_t bucket_per_subtree = 2U << SubtreeDepth;

  using subtree = std::array<bucket_type, bucket_per_subtree>;

  using subtrees = std::array<subtree, id::bit_size>;

  struct find_bucket_result final
  {
    std::size_t subtree_index;
    std::size_t bucket_index;
  };

  friend std::formatter<routing_table>;

private:
  [[nodiscard]] constexpr id distance(id const& peer_id) const noexcept
  {
    return my_id_ ^ peer_id;
  }

  static constexpr find_bucket_result find_bucket(id const& peer_distance) noexcept
  {
    find_bucket_result result{
      .subtree_index = peer_distance.countl_zero(),
      .bucket_index = {},
    };

    assert(result.subtree_index < id::bit_size);

    for (auto value : peer_distance |
                          std::ranges::views::drop(result.subtree_index) |
                          std::ranges::views::take(bucket_per_subtree)) {
      result.bucket_index <<= 1U;
      result.bucket_index += bool(value);
    }

    assert(result.bucket_index < bucket_per_subtree);

    return result;
  }

private:
  id my_id_;
  std::unique_ptr<subtrees> subtrees_{std::make_unique<subtrees>()};
};

} // namespace ks::dht::abiv1::detail

template<typename PeerType, std::size_t BucketSize, std::size_t SubtreeDepth>
struct std::formatter<
    ks::dht::detail::routing_table<PeerType, BucketSize, SubtreeDepth>> final
{
  constexpr auto parse(auto& ctx) const { return ctx.begin(); }

  auto format(auto const& routing_table, auto& ctx) const
  {
    ctx.advance_to(std::format_to(ctx.out(),
                                  R"({{"id":"{}","peer_count":{},)"
                                  R"("k_bucket_size":{}}})",
                                  routing_table.my_id_,
                                  routing_table.peer_count_,
                                  routing_table.k_bucket_size_));
    return ctx.out();
  }
};
