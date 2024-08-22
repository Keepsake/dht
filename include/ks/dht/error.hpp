// SPDX-License-Identifier: MIT

#pragma once

#include <system_error>

#include <ks/dht/detail/symbol_visibility.hpp>

namespace ks::dht {
inline namespace abiv1 {

/// This enum list all library specific errors.
enum class error
{
  /// An unknown error.
  unknown_error = 1,
  /// The session::abort() has been called.
  run_aborted,
  /// The session failed to contact a valid peer uppon creation.
  initial_peer_failed_to_respond,
  /// The session routing table is missing peers.
  missing_peers,
  /// An id has been corrupted.
  invalid_id,
  /// An id has been truncated.
  truncated_id,
  /// A packet header from the network is corrupted.
  truncated_header,
  /// An endpoint information has been corrupted.
  truncated_endpoint,
  /// An endpoint address has been corrupted.
  truncated_address,
  /// A list has been corrupted.
  truncated_size,
  /// A message from an unknown version of the library has been received.
  unknown_protocol_version,
  /// A packet body has been corrupted.
  corrupted_body,
  /// An unexpected response has been received.
  unassociated_message_id,
  /// The provided IPv4 address is invalid.
  invalid_ipv4_address,
  /// The provided IPv6 address is invalid.
  invalid_ipv6_address,
  /// The function/method has been implemented yet.
  unimplemented,
  /// The value associated with the requested key has not been found.
  value_not_found,
  /// The internal timer failed to tick.
  timer_malfunction,
  /// Another call to session::run() is still blocked.
  already_running,
};

KS_DHT_EXPORT std::error_category const&
error_category() noexcept;

std::error_condition
make_error_condition(error error) noexcept
{
  return { static_cast<int>(error), error_category() };
}

namespace detail {

std::error_code
make_error_code(error error) noexcept
{
  return { static_cast<int>(error), error_category() };
}

} // namespace detail

} // namespace abiv1
} // namespace ks::dht

template<>
struct std::is_error_condition_enum<ks::dht::error> : true_type
{};
