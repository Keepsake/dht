// SPDX-License-Identifier: MIT

#include <ks/dht/error.hpp>

#include <string>

namespace ks::dht {
inline namespace abiv1 {

namespace {

/**
 *
 */
struct dht_error_category final : std::error_category
{
  char const* name(void) const noexcept override { return "dht"; }

  std::string message(int condition) const noexcept override
  {
    switch (condition) {
      case RUN_ABORTED:
        return "run aborted";
      case INITIAL_PEER_FAILED_TO_RESPOND:
        return "initial peer failed to respond";
      case MISSING_PEERS:
        return "missing peers";
      case UNIMPLEMENTED:
        return "unimplemented";
      case INVALID_ID:
        return "invalid id";
      case TRUNCATED_ID:
        return "truncated id";
      case TRUNCATED_ENDPOINT:
        return "truncated endpoint";
      case TRUNCATED_ADDRESS:
        return "truncated address";
      case TRUNCATED_HEADER:
        return "truncated header";
      case TRUNCATED_SIZE:
        return "truncated size";
      case CORRUPTED_BODY:
        return "corrupted body";
      case UNKNOWN_PROTOCOL_VERSION:
        return "unknown protocol version";
      case UNASSOCIATED_MESSAGE_ID:
        return "unassociated message id";
      case INVALID_IPV4_ADDRESS:
        return "invalid IPv4 address";
      case INVALID_IPV6_ADDRESS:
        return "invalid IPv6 address";
      case VALUE_NOT_FOUND:
        return "value not found";
      case TIMER_MALFUNCTION:
        return "timer malfunction";
      case ALREADY_RUNNING:
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
  static dht_error_category const category_{};
  return category_;
}

} // namespace abiv1
} // namespace ks::dht
