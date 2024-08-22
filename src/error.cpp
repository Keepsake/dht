// SPDX-License-Identifier: MIT

#include <ks/dht/error.hpp>

#include <string>

namespace ks::dht {
inline namespace abiv1 {

namespace {

/**
 *
 */
struct dht_category final : std::error_category
{
  char const* name() const noexcept override { return "dht"; }

  std::string message(int condition) const noexcept override
  {
    switch (static_cast<error>(condition)) {
      case error::run_aborted:
        return "run aborted";
      case error::initial_peer_failed_to_respond:
        return "initial peer failed to respond";
      case error::missing_peers:
        return "missing peers";
      case error::unimplemented:
        return "unimplemented";
      case error::invalid_id:
        return "invalid id";
      case error::truncated_id:
        return "truncated id";
      case error::truncated_endpoint:
        return "truncated endpoint";
      case error::truncated_address:
        return "truncated address";
      case error::truncated_header:
        return "truncated header";
      case error::truncated_size:
        return "truncated size";
      case error::corrupted_body:
        return "corrupted body";
      case error::unknown_protocol_version:
        return "unknown protocol version";
      case error::unassociated_message_id:
        return "unassociated message id";
      case error::invalid_ipv4_address:
        return "invalid ipv4 address";
      case error::invalid_ipv6_address:
        return "invalid ipv6 address";
      case error::value_not_found:
        return "value not found";
      case error::timer_malfunction:
        return "timer malfunction";
      case error::already_running:
        return "already running";
      default:
        return "unknown error";
    }
  }
};

} // namespace

std::error_category const&
error_category() noexcept
{
  static dht_category const category_{};
  return category_;
}

} // namespace abiv1
} // namespace ks::dht
