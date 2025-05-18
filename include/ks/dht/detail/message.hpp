// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <system_error>
#include <variant>
#include <vector>

#include <ks/serialization/serialize.hpp>
#include <ks/serialization/std.hpp>

#include <ks/dht/detail/id.hpp>
#include <ks/dht/detail/peer.hpp>
#include <ks/dht/error.hpp>

namespace ks::dht {
inline namespace abiv1 {
namespace detail {

inline constexpr std::uint8_t protocol_version{ 1U };

struct message_header final
{
  id source_id;
  id random_token;

  bool operator==(message_header const&) const noexcept = default;
};

struct ping_request_body final
{
  bool operator==(ping_request_body const&) const noexcept = default;
};

struct ping_response_body final
{
  bool operator==(ping_response_body const&) const noexcept = default;
};

struct find_peer_request_body final
{
  id peer_to_find;

  bool operator==(find_peer_request_body const&) const noexcept = default;
};

struct find_peer_response_body final
{
  std::vector<peer> peers;

  bool operator==(find_peer_response_body const&) const noexcept = default;
};

struct find_value_request_body final
{
  id value_to_find;

  bool operator==(find_value_request_body const&) const noexcept = default;
};

struct find_value_response_body final
{
  std::vector<std::byte> data;

  bool operator==(find_value_response_body const&) const noexcept = default;
};

struct store_value_request_body final
{
  id data_key_hash;
  std::vector<std::byte> data_value;

  bool operator==(store_value_request_body const&) const noexcept = default;
};

using message_body = std::variant<ping_request_body,
                                  ping_response_body,
                                  find_peer_request_body,
                                  find_peer_response_body,
                                  find_value_request_body,
                                  find_value_response_body,
                                  store_value_request_body>;

struct message final
{
  message_header header;
  message_body body;

  bool operator==(message const&) const noexcept = default;
};

struct const_message_view final
{
  message_header const& header;
  message_body const& body;
};

struct message_view final
{
  message_header & header;
  message_body & body;
};

} // namespace detail
} // namespace abiv1
} // namespace ks::dht

template<>
struct ks::serialization::serializer<ks::dht::detail::message_header> final
{
  std::error_code operator()(ks::serialization::oarchive& archive,
                             auto const& header) const
  {
    return serialize(archive,
                     ks::dht::detail::protocol_version,
                     header.source_id,
                     header.random_token);
  }

  std::error_code operator()(ks::serialization::iarchive& archive,
                             auto& header) const
  {
    std::uint8_t version{};
    if (auto failure = serialize(archive, version); failure) [[unlikely]]
      return failure;

    if (version != ks::dht::detail::protocol_version) [[unlikely]]
      return ks::dht::error::unknown_protocol_version;

    return serialize(archive, header.source_id, header.random_token);
  }
};

template<>
struct ks::serialization::serializer<ks::dht::detail::ping_request_body> final
{
  std::error_code operator()(auto& archive, auto& body) const { return {}; }
};

template<>
struct ks::serialization::serializer<ks::dht::detail::ping_response_body> final
{
  std::error_code operator()(auto& archive, auto& body) const { return {}; }
};

template<>
struct ks::serialization::serializer<
    ks::dht::detail::find_peer_request_body> final
{
  std::error_code operator()(auto& archive, auto& body) const
  {
    return serialize(archive, body.peer_to_find);
  }
};

template<>
struct ks::serialization::serializer<
    ks::dht::detail::find_peer_response_body> final
{
  std::error_code operator()(auto& archive, auto& body) const
  {
    return serialize(archive, body.peers);
  }
};

template<>
struct ks::serialization::serializer<
    ks::dht::detail::find_value_request_body> final
{
  std::error_code operator()(auto& archive, auto& body) const
  {
    return serialize(archive, body.value_to_find);
  }
};

template<>
struct ks::serialization::serializer<
    ks::dht::detail::find_value_response_body> final
{
  std::error_code operator()(auto& archive, auto& body) const
  {
    return serialize(archive, body.data);
  }
};

template<>
struct ks::serialization::serializer<
    ks::dht::detail::store_value_request_body> final
{
  std::error_code operator()(auto& archive, auto& body) const
  {
    return serialize(archive, body.data_key_hash, body.data_value);
  }
};

template<>
struct ks::serialization::serializer<ks::dht::detail::message> final
{
  std::error_code operator()(auto& archive, auto& message) const
  {
    return serialize(archive, message.header, message.body);
  }
};

template<>
struct ks::serialization::serializer<ks::dht::detail::const_message_view> final
{
  std::error_code operator()(auto& archive, auto& message) const
  {
    return serialize(archive, message.header, message.body);
  }
};

template<>
struct ks::serialization::serializer<ks::dht::detail::message_view> final
{
  std::error_code operator()(auto& archive, auto& message) const
  {
    return serialize(archive, message.header, message.body);
  }
};

template<>
struct std::formatter<ks::dht::detail::message_header> final
{
  constexpr auto parse(auto& ctx) const { return ctx.begin(); }

  constexpr auto format(ks::dht::detail::message_header const& header,
                        auto& ctx) const
  {
    return std::format_to(ctx.out(),
                          R"(message_header{"source": "{}", "token": "{}"})",
                          header.source_id,
                          header.random_token);
  }
};
