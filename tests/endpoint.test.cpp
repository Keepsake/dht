// SPDX-License-Identifier: MIT

#include <format>

#include <gtest/gtest.h>

#include <ks/dht/endpoint.hpp>

namespace k = ks::dht;

TEST(endpoint_test, can_be_printed)
{
  {
    k::endpoint_v4 const e{
      .ip = asio::ip::make_address_v4("127.0.0.1"),
      .port = 1234,
    };
    ASSERT_EQ(std::format("{}", e), "127.0.0.1:1234");
  }
  {
    k::endpoint const e{ k::endpoint_v4{
        .ip = asio::ip::make_address_v4("127.0.0.1"),
        .port = 1234,
    } };
    ASSERT_EQ(std::format("{}", e), "127.0.0.1:1234");
  }
  {
    k::endpoint_v6 const e{
      .ip = asio::ip::make_address_v6("::1"),
      .port = 1234,
    };
    ASSERT_EQ(std::format("{}", e), "::1:1234");
  }
  {
    k::endpoint const e{ k::endpoint_v6{
        .ip = asio::ip::make_address_v6("::1"),
        .port = 1234,
    } };
    ASSERT_EQ(std::format("{}", e), "::1:1234");
  }
}

TEST(endpoint_test, can_serialize_v4)
{
  k::endpoint_v4 const endpoint_out{
    .ip = asio::ip::make_address_v4("127.0.0.1"),
    .port = 1234,
  };

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, endpoint_out));

  k::endpoint_v4 endpoint_in;
  ASSERT_FALSE(ks::serialization::load(buffer, endpoint_in));

  ASSERT_EQ(endpoint_in, endpoint_out);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, endpoint_in),
              ks::serialization::error::buffer_underrun);
  }
}

TEST(endpoint_test, can_serialize_variant_v4)
{
  k::endpoint const endpoint_out{ k::endpoint_v4{
      .ip = asio::ip::make_address_v4("127.0.0.1"),
      .port = 1234,
  } };

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, endpoint_out));

  k::endpoint endpoint_in;
  ASSERT_FALSE(ks::serialization::load(buffer, endpoint_in));

  ASSERT_EQ(endpoint_in, endpoint_out);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, endpoint_in),
              ks::serialization::error::buffer_underrun);
  }
}

TEST(endpoint_test, can_serialize_v6)
{
  k::endpoint_v6 const endpoint_out{
    .ip = asio::ip::make_address_v6("::1"),
    .port = 1234,
  };

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, endpoint_out));

  k::endpoint_v6 endpoint_in;
  ASSERT_FALSE(ks::serialization::load(buffer, endpoint_in));

  ASSERT_EQ(endpoint_in, endpoint_out);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, endpoint_in),
              ks::serialization::error::buffer_underrun);
  }
}

TEST(endpoint_test, can_serialize_variant_v6)
{
  k::endpoint const endpoint_out{ k::endpoint_v6{
      .ip = asio::ip::make_address_v6("::1"),
      .port = 1234,
  } };

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, endpoint_out));

  k::endpoint endpoint_in;
  ASSERT_FALSE(ks::serialization::load(buffer, endpoint_in));

  ASSERT_EQ(endpoint_in, endpoint_out);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, endpoint_in),
              ks::serialization::error::buffer_underrun);
  }
}
