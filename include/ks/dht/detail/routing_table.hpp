// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <memory>
#include <cassert>
#include <cstdint>
#include <format>
#include <list>
#include <utility>
#include <vector>

#include <ks/dht/detail/id.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

/**
 *  This class keeps track of peers and find the known peer closed to an id.
 *  @note Current implementation use a discret symbol approach.
 */
template<typename PeerType>
class routing_table final
{
public:
  static constexpr std::size_t default_k_bucket_size = 20;

  ///
  using peer_type = PeerType;

  ///
  using value_type = std::pair<id, peer_type>;

  class iterator;

public:
  /**
   *  Construct the routing_table implementation.
   */
  constexpr routing_table(id const& my_id,
                std::size_t k_bucket_size = default_k_bucket_size)
    : k_buckets_(id::bit_size)
    , my_id_(my_id)
    , k_bucket_size_(k_bucket_size)
  {
    assert(k_bucket_size_ > 0 and "k_bucket size must be > 0");
  }

  /**
   *  Disabled copy constructor.
   */
  routing_table(routing_table const&) = delete;

  /**
   *  Disabled assignement operator.
   */
  routing_table& operator=(routing_table const&) = delete;

  /**
   *  Count the number of peer in the routing table.
   *  @note Complexity: O(1).
   */
  constexpr std::size_t peer_count() const noexcept { return peer_count_; }

  /**
   *  Register a peer into the routing table.
   *  @return true if the peer has been inserted.
   *  @note This method takes ownership of the peer.
   *  @note The peer may not be pushed if the target bucket is full.
   *  @note Complexity: O(log n)
   */
  bool push(id const& peer_id, peer_type const& new_peer)
  {
    auto k_bucket_index = find_k_bucket_index(peer_id);
    auto& bucket = k_buckets_[k_bucket_index];

    // If there is room in the bucket.
    if (bucket.size() == k_bucket_size_) {
      update_largest_k_bucket_index(k_bucket_index);

      if (k_bucket_index != largest_k_bucket_index_)
        return false;
    }

    auto const end = bucket.end();

    // Check if the peer is not already known.
    auto is_peer_known = [&peer_id](value_type const& entry) {
      return entry.first == peer_id;
    };

    if (std::find_if(bucket.begin(), end, is_peer_known) != end)
      return false;

    bucket.insert(end, value_type{ peer_id, new_peer });
    ++peer_count_;

    return true;
  }

  /**
   *  Remove a peer from the routing table.
   *  @return true if the peer has been removed.
   *  @note Complexity: O(log n)
   */
  bool remove(id const& peer_id)
  {
    // Find the closer bucket.
    auto& bucket = k_buckets_[find_k_bucket_index(peer_id)];

    // Check if the peer is inside.
    auto is_peer_known = [&peer_id](value_type const& entry) {
      return entry.first == peer_id;
    };

    auto i = std::find_if(bucket.begin(), bucket.end(), is_peer_known);

    // If the peer wasn't inside.
    if (i == bucket.end())
      return false;

    // Remove it.
    bucket.erase(i);
    --peer_count_;

    return true;
  }

  /**
   *  Find closest peers to an id.
   *  @return An iterator to the closest peer from the id to the far.
   *  @note Complexity: O(log n)
   */
  iterator find(id const& id_to_find)
  {
    auto index =
        std::max(get_lowest_k_bucket_index(), find_k_bucket_index(id_to_find));

    auto i = std::next(k_buckets_.begin(), index);

    // Find the first non empty k_bucket.
    while (i->empty() and i != k_buckets_.begin())
      --i;

    return iterator{&k_buckets_, i, i->begin()};
  }

  /**
   *  @return An iterator to the end of the routing table.
   */
  iterator end()
  {
    assert(k_buckets_.size() > 0 and
           "routing_table must always contains k_buckets");
    auto const first_k_bucket = k_buckets_.begin();

    return iterator{&k_buckets_, first_k_bucket, first_k_bucket->end()};
  }

private:
  /// Contains peer with a common base id.
  using k_bucket = std::list<value_type>;
  /// Contains all the k_bucket.
  /// @note Algorithms expect a vector here, do not change this.
  using k_buckets = std::vector<k_bucket>;

  friend std::formatter<routing_table>;

private:
  /**
   *
   */
  constexpr std::size_t find_k_bucket_index(id const& id_to_find) const noexcept
  {
    // Find closest bucket from the peer id.
    // i.e. the index of the first different bit
    // in the id of the new peer vs our id is equal to the
    // index of the closest bucket in the buckets container.
    std::size_t bit_index{};
    while (bit_index < id::bit_size - 1 and
           id_to_find[bit_index] == my_id_[bit_index])
      ++bit_index;

    return bit_index;
  }

  /**
   *
   */
  constexpr std::size_t get_lowest_k_bucket_index() const noexcept
  {
    std::size_t i{};
    std::size_t e{k_buckets_.size() - 1};

    for (std::size_t peer_count{}; i != e and peer_count <= k_bucket_size_;
         ++i)
      peer_count += k_buckets_[i].size();

    return i;
  }

  /**
   *
   */
  void update_largest_k_bucket_index(std::size_t index)
  {
    if (k_buckets_[largest_k_bucket_index_].size() <= k_bucket_size_)
      largest_k_bucket_index_ = index;
  }

private:
  /// This contains buckets up to id bit count.
  k_buckets k_buckets_;
  /// Own id.
  id const my_id_;
  /// Keep a track of peer count to make size() complexity O(1).
  std::size_t peer_count_{};
  /// This is max number of peers stored per k_bucket.
  std::size_t k_bucket_size_;
  /// This keeps the index of the largest subtree.
  std::size_t largest_k_bucket_index_{};
};

/**
 *
 */
template<typename PeerType>
class routing_table<PeerType>::iterator final
{
public:
  using difference_type = std::ptrdiff_t;

  using value_type = typename routing_table::value_type;

  using reference = value_type &;

  using pointer = value_type *;

  using iterator_category = std::forward_iterator_tag;

public:
  /**
   *
   */
  constexpr iterator(k_buckets* buckets,
           typename k_buckets::iterator current_bucket,
           typename k_bucket::iterator current_peer)
    : k_buckets_(buckets)
    , current_k_bucket_(current_bucket)
    , current_entry_(current_peer)
  {
  }

  constexpr iterator(iterator const& o) = default;
  constexpr iterator& operator=(iterator const& o) = default;

  constexpr bool operator==(iterator const& o) const noexcept = default;

  constexpr iterator& operator++() noexcept
  {
    ++current_entry_;

    // If the current entry is not at the end of the bucket
    // then there is nothing more to do.
    if (current_entry_ != current_k_bucket_->end())
      return *this;

    // If the current bucket is already the first (far)
    // then there is nothing more to do, we reach the end of the routing table.
    if (current_k_bucket_ == k_buckets_->begin())
      return *this;

    // Go to the next non-empty bucket and start from its first entry.
    do
      --current_k_bucket_;
    while (current_k_bucket_->empty() and
           current_k_bucket_ != k_buckets_->begin());
    current_entry_ = current_k_bucket_->begin();

    return *this;
  }

  constexpr iterator& operator++(int) noexcept
  {
    auto old{*this};
   
    operator++();

    return old;
  }

  reference operator*() const noexcept
  {
    return *current_entry_;
  }

  pointer operator->() const noexcept
  {
    return std::addressof(operator*());
  }

private:
  k_buckets* k_buckets_;
  typename k_buckets::iterator current_k_bucket_;
  typename k_bucket::iterator current_entry_;
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht

template<typename PeerType>
struct std::formatter<ks::dht::detail::routing_table<PeerType>> final
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
