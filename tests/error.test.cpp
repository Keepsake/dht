// Copyright (c) 2013-2014, David Keller
// All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the University of California, Berkeley nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY DAVID KELLER AND CONTRIBUTORS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>

#include <gtest/gtest.h>

#include <ks/dht/error.hpp>

namespace k = ks::dht;

bool
compare_enum_to_message(char const* name, k::error const& error)
{
  auto message = make_error_code(error).message();

  std::replace(message.begin(), message.end(), ' ', '_');

  return name == message;
}

#define DHT_TEST_ERROR(e) ASSERT_TRUE(compare_enum_to_message(#e, k::error::e))

TEST(error_test_usage, error_message_follows_the_error_name)
{
  DHT_TEST_ERROR(unknown_error);
  DHT_TEST_ERROR(run_aborted);
  DHT_TEST_ERROR(initial_peer_failed_to_respond);
  DHT_TEST_ERROR(missing_peers);
  DHT_TEST_ERROR(invalid_id);
  DHT_TEST_ERROR(truncated_id);
  DHT_TEST_ERROR(truncated_header);
  DHT_TEST_ERROR(truncated_endpoint);
  DHT_TEST_ERROR(truncated_address);
  DHT_TEST_ERROR(truncated_size);
  DHT_TEST_ERROR(unknown_protocol_version);
  DHT_TEST_ERROR(corrupted_body);
  DHT_TEST_ERROR(unassociated_message_id);
  DHT_TEST_ERROR(invalid_ipv4_address);
  DHT_TEST_ERROR(invalid_ipv6_address);
  DHT_TEST_ERROR(unimplemented);
  DHT_TEST_ERROR(value_not_found);
  DHT_TEST_ERROR(timer_malfunction);
  DHT_TEST_ERROR(already_running);
}

#undef DHT_TEST_ERROR

TEST(error_test_usage, error_category_is_dht)
{
  auto e = make_error_code(k::error::unknown_error);
  ASSERT_STREQ("dht", e.category().name());
}
