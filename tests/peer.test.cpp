// SPDX-License-Identifier: MIT

#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include <ks/dht/detail/peer.hpp>

namespace k = ks::dht;
namespace kd = k::detail;

TEST(peer_test_usage, can_be_printed)
{
  kd::peer const peer{ .id{ "0123456789abcdef" },
                       .endpoint{ k::endpoint_v4{
                           asio::ip::make_address_v4("127.0.0.1"),
                           1234,
                       } } };

  ASSERT_EQ(std::format("{}", peer), "0123456789abcdef/127.0.0.1:1234");
}

TEST(peer_test_usage, can_be_serialized)
{
  kd::peer const peer_out{ .id{ "0123456789abcdef" },
                           .endpoint{ k::endpoint_v4{
                               asio::ip::make_address_v4("127.0.0.1"),
                               1234,
                           } } };

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, peer_out));

  kd::peer peer_in;
  ASSERT_FALSE(ks::serialization::load(buffer, peer_in));

  ASSERT_EQ(peer_in, peer_out);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, peer_in),
              ks::serialization::error::buffer_underrun);
  }
}
