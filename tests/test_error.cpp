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
#include <cctype>

#include <gtest/gtest.h>

#include <ks/dht/error.hpp>

#include "common.hpp"

namespace k = ks::dht;

bool
compare_enum_to_message(char const* name, k::error_type const& error)
{
  auto message = make_error_condition(error).message();

  std::replace(message.begin(), message.end(), ' ', '_');
  std::transform(message.begin(), message.end(), message.begin(), ::toupper);

  return name == message;
}

#define DHT_TEST_ERROR(e) ASSERT_TRUE(compare_enum_to_message(#e, k::e))

TEST(error_test_usage, error_message_follows_the_error_name)
{
  DHT_TEST_ERROR(UNKNOWN_ERROR);
  DHT_TEST_ERROR(RUN_ABORTED);
  DHT_TEST_ERROR(INITIAL_PEER_FAILED_TO_RESPOND);
  DHT_TEST_ERROR(MISSING_PEERS);
  DHT_TEST_ERROR(INVALID_ID);
  DHT_TEST_ERROR(TRUNCATED_ID);
  DHT_TEST_ERROR(TRUNCATED_HEADER);
  DHT_TEST_ERROR(TRUNCATED_ENDPOINT);
  DHT_TEST_ERROR(TRUNCATED_ADDRESS);
  DHT_TEST_ERROR(TRUNCATED_SIZE);
  DHT_TEST_ERROR(UNKNOWN_PROTOCOL_VERSION);
  DHT_TEST_ERROR(CORRUPTED_BODY);
  DHT_TEST_ERROR(UNASSOCIATED_MESSAGE_ID);
  DHT_TEST_ERROR(INVALID_IPV4_ADDRESS);
  DHT_TEST_ERROR(INVALID_IPV6_ADDRESS);
  DHT_TEST_ERROR(UNIMPLEMENTED);
  DHT_TEST_ERROR(VALUE_NOT_FOUND);
  DHT_TEST_ERROR(TIMER_MALFUNCTION);
  DHT_TEST_ERROR(ALREADY_RUNNING);
}

TEST(error_test_usage, error_category_is_dht)
{
  auto e = make_error_condition(k::UNKNOWN_ERROR);
  ASSERT_STREQ("dht", e.category().name());
}

#undef DHT_TEST_ERROR
