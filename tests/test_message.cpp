// SPDX-License-Identifier: MIT

#include <fstream>
#include <random>
#include <sstream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ks/serialization.hpp>

#include <ks/dht/detail/message.hpp>
#include <ks/dht/error.hpp>

#include "common.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

TEST(message_test_message, can_detect_corrupted_header)
{
  std::default_random_engine random_engine;

  // Wrong version.
  {
    kd::message_header const header_out{
      .source_id{ random_engine },
      .random_token{ random_engine },
    };

    std::vector<std::byte> buffer;
    ASSERT_FALSE(ks::serialization::save(buffer, header_out));

    buffer.at(0) = std::byte{ 64 };

    kd::message_header header_in;
    ASSERT_EQ(ks::serialization::load(buffer, header_in),
              k::error::unknown_protocol_version);
  }

  // Missing bytes.
  {
    kd::message_header const header_out{
      .source_id{ random_engine },
      .random_token{ random_engine },
    };

    std::vector<std::byte> buffer;
    ASSERT_FALSE(ks::serialization::save(buffer, header_out));

    kd::message_header header_in;
    while (not buffer.empty()) {
      buffer.pop_back();
      ASSERT_EQ(ks::serialization::load(buffer, header_in),
                ks::serialization::error::buffer_underrun);
    }
  }
}

TEST(message_test_message, can_serialize_header)
{
  std::default_random_engine random_engine;

  kd::message_header const header_out{
    .source_id{ random_engine },
    .random_token{ random_engine },
  };

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, header_out));

  kd::message_header header_in;
  ASSERT_FALSE(ks::serialization::load(buffer, header_in));

  ASSERT_EQ(header_out.source_id, header_in.source_id);
  ASSERT_EQ(header_out.random_token, header_in.random_token);
}

TEST(message_test_message, can_serialize_find_peer_request_body)
{
  std::default_random_engine random_engine;

  kd::find_peer_request_body const body_out{
    .peer_to_find{ random_engine },
  };

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, body_out));

  kd::find_peer_request_body body_in;
  ASSERT_FALSE(ks::serialization::load(buffer, body_in));

  ASSERT_EQ(body_out.peer_to_find, body_in.peer_to_find);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, body_in),
              ks::serialization::error::buffer_underrun);
  }
}

TEST(message_test_message, can_serialize_find_peer_response_body)
{
  std::default_random_engine random_engine;

  kd::find_peer_response_body body_out;

  for (std::size_t i = 0; i < 10; ++i) {
    kd::peer new_peer{ .id{ random_engine },
                       .endpoint = k::endpoint_v4{
                           .ip = asio::ip::make_address_v4("127.0.0.1"),
                           .port = std::uint16_t(1024 + i),
                       } };

    body_out.peers.push_back(std::move(new_peer));
  }

  std::vector<std::byte> buffer;
  ks::serialization::save(buffer, body_out);

  kd::find_peer_response_body body_in;
  ASSERT_FALSE(ks::serialization::load(buffer, body_in));

  ASSERT_EQ(body_out, body_in);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, body_in),
              ks::serialization::error::buffer_underrun);
  }
}

TEST(message_test_message, can_serialize_find_value_request_body)
{
  std::default_random_engine random_engine;

  kd::find_value_request_body const body_out{
    kd::id{ random_engine },
  };

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, body_out));

  kd::find_value_request_body body_in;
  ASSERT_FALSE(ks::serialization::load(buffer, body_in));

  ASSERT_EQ(body_out, body_in);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, body_in),
              ks::serialization::error::buffer_underrun);
  }
}

TEST(message_test_message, can_serialize_find_value_response_body)
{
  kd::find_value_response_body body_out{ std::vector<std::byte>(4096) };
  std::ranges::generate(body_out.data,
                        [] { return static_cast<std::byte>(std::rand()); });

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, body_out));

  kd::find_value_response_body body_in;
  ASSERT_FALSE(ks::serialization::load(buffer, body_in));

  ASSERT_EQ(body_out, body_in);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, body_in),
              ks::serialization::error::buffer_underrun);
  }
}

TEST(message_test_message, can_serialize_store_value_request_body)
{
  std::default_random_engine random_engine;

  kd::store_value_request_body body_out{
    .data_key_hash{ random_engine },
    .data_value = std::vector<std::byte>(4096),
  };

  std::ranges::generate(body_out.data_value,
                        [] { return static_cast<std::byte>(std::rand()); });

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, body_out));

  kd::store_value_request_body body_in;
  ASSERT_FALSE(ks::serialization::load(buffer, body_in));

  ASSERT_EQ(body_out, body_in);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, body_in),
              ks::serialization::error::buffer_underrun);
  }
}

TEST(message_test_message, can_serialize_message)
{
  std::default_random_engine random_engine;

  kd::message message_out{
    .header{
      .source_id{ random_engine },
      .random_token{ random_engine },
    },
    .body = kd::find_peer_request_body{
      .peer_to_find{ random_engine },
    }
  };

  std::vector<std::byte> buffer;
  ASSERT_FALSE(ks::serialization::save(buffer, message_out));

  kd::message message_in;
  ASSERT_FALSE(ks::serialization::load(buffer, message_in));

  ASSERT_EQ(message_out, message_in);

  while (not buffer.empty()) {
    buffer.pop_back();
    ASSERT_EQ(ks::serialization::load(buffer, message_in),
              ks::serialization::error::buffer_underrun);
  }
}
