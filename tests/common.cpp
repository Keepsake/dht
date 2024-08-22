// SPDX-License-Identifier: MIT

#include "common.hpp"

#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <asio/ip/udp.hpp>

namespace {

std::filesystem::path const tests_directory_{ TESTS_DIR };

}

std::string
get_capture_path(std::string const& capture_name)
{
  return (tests_directory_ / "captures" / capture_name).string();
}

void
check_listening(std::string const& ip, std::uint16_t port)
{
  using asio::ip::udp;
  auto udp_failure = create_socket<udp::socket>(ip, port);
  ASSERT_EQ(std::errc::address_in_use, udp_failure);
}

std::uint16_t
get_temporary_listening_port(std::uint16_t port)
{
  std::error_code failure;

  do {
    ++port;
    asio::ip::udp::endpoint const e{ asio::ip::udp::v4(), port };

    asio::io_context io_context;
    // Try to open a socket at this address.
    asio::ip::udp::socket socket{ io_context, e.protocol() };
    socket.bind(e, failure);
  } while (failure == std::errc::address_in_use);

  return port;
}
