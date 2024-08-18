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

#include <system_error>

#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>

#include <gtest/gtest.h>

#include "boost_to_std_error.hpp"
#include "common.hpp"

namespace kd = ks::dht::detail;
namespace ba = boost::asio;
namespace bs = boost::system;

TEST(boost_to_std_error_test_usage, can_convert_generic_error)
{
  auto const c = make_error_code(bs::errc::address_in_use);
  auto const e = make_error_code(std::errc::address_in_use);
  ASSERT_EQ(kd::boost_to_std_error(c), e);
}

TEST(boost_to_std_error_test_usage, can_convert_system_error)
{
  bs::error_code const c{ 1000, bs::system_category() };
  std::error_code const e{ 1000, std::system_category() };
  ASSERT_EQ(kd::boost_to_std_error(c), e);
}

TEST(boost_to_std_error_test_usage, cannot_convert_asio_error)
{
  bs::error_code const c{ 1000, ba::error::misc_category };
  ASSERT_EQ(kd::boost_to_std_error(c), ks::dht::UNKNOWN_ERROR);
}
