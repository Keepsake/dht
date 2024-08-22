// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <system_error>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>
#include <asio/ip/v6_only.hpp>

std::string
get_capture_path(std::string const& capture_name);

template<typename Socket>
std::error_code
create_socket(std::string const& ip, std::uint16_t port)
{
  auto const a = asio::ip::address::from_string(ip);
  asio::io_context io_context;

  // Try to create a socket.
  typename Socket::endpoint_type endpoint(a, port);
  Socket socket{ io_context, endpoint.protocol() };

  if (endpoint.address().is_v6())
    socket.set_option(asio::ip::v6_only{ true });

  std::error_code failure;
  socket.bind(endpoint, failure);

  return failure;
}

void
check_listening(std::string const& ip, std::uint16_t port);

std::uint16_t
get_temporary_listening_port(std::uint16_t port = 1234);
