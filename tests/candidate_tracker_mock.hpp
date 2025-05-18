// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ks/dht/detail/id.hpp>
#include <ks/dht/detail/peer.hpp>

struct candidate_tracker_mock final
{
  MOCK_METHOD(std::vector<ks::dht::detail::peer>,
              select_new_candidates,
              (std::size_t max_count));
  MOCK_METHOD(void,
              add_candidates,
              (std::vector<ks::dht::detail::peer> const& peers));
  MOCK_METHOD(ks::dht::detail::id, get_key, (), (const noexcept));
};
