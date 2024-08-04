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

#include <sstream>

#include <gtest/gtest.h>

#include <ks/dht/endpoint.hpp>

#include "common.hpp"

namespace k = ks::dht;

TEST( endpoint_test_construction, can_be_constructed_with_service_as_string )
{
    k::endpoint{ "127.0.0.1", "1234" };
}

TEST( endpoint_test_construction, can_be_constructed_with_service_as_integer )
{
    k::endpoint{ "127.0.0.1", 1234 };
}

TEST( endpoint_test_getter_and_setters, can_be_inspected_and_modified )
{
    k::endpoint e{ "127.0.0.1", "1234" };
    ASSERT_EQ( "127.0.0.1", e.address() );
    ASSERT_EQ( "1234", e.service() );

    e.address( "192.168.0.1" );
    e.service( "4567" );
    ASSERT_EQ( "192.168.0.1", e.address() );
    ASSERT_EQ( "4567", e.service() );
}

TEST( endpoint_test_getter_and_setters, can_be_printed )
{
    std::ostringstream out;

    out << k::endpoint{ "127.0.0.1", 1234 };

    ASSERT_EQ( out.str(), "127.0.0.1:1234" );
}

TEST( endpoint_test_getter_and_setters, can_be_parsed )
{
    // IPv4 + numeric port
    {
        k::endpoint e;
        std::istringstream in{ "127.0.0.1:1234" };
        in >> e;
        ASSERT_FALSE( in.fail() );
        ASSERT_EQ( "127.0.0.1", e.address() );
        ASSERT_EQ( "1234", e.service() );
    }

    // IPv6 + numeric port
    {
        k::endpoint e;
        std::istringstream in{ "[AA:bb::1]:1234" };
        in >> e;
        ASSERT_FALSE( in.fail() );
        ASSERT_EQ( "AA:bb::1", e.address() );
        ASSERT_EQ( "1234", e.service() );
    }

    // IPv4 + named port
    {
        k::endpoint e;
        std::istringstream in{ "127.0.0.1:http" };
        in >> e;
        ASSERT_FALSE( in.fail() );
        ASSERT_EQ( "127.0.0.1", e.address() );
        ASSERT_EQ( "http", e.service() );
    }

    // IPv6 + named port
    {
        k::endpoint e;
        std::istringstream in{ "[AA:bb::1]:http" };
        in >> e;
        ASSERT_FALSE( in.fail() );
        ASSERT_EQ( "AA:bb::1", e.address() );
        ASSERT_EQ( "http", e.service() );
    }

    // Invalid because missing address/service separator
    // Expected the provided endpoint to remaining unmodified
    {
        k::endpoint e{ "0.0.0.0", "http" };
        std::istringstream in{ "192.168.0.1http" };
        in >> e;
        ASSERT_TRUE( in.fail() );
        ASSERT_EQ( "0.0.0.0", e.address() );
        ASSERT_EQ( "http", e.service() );
    }

    // Invalid because missing IPv6 leading bracket separator
    // Expected the provided endpoint to remaining unmodified
    {
        k::endpoint e{ "0.0.0.0", "http" };
        std::istringstream in{ "AA:bb::1]:http" };
        in >> e;
        ASSERT_TRUE( in.fail() );
        ASSERT_EQ( "0.0.0.0", e.address() );
        ASSERT_EQ( "http", e.service() );
    }

    // Invalid because missing IPv6 trailing bracket separator
    // Expected the provided endpoint to remaining unmodified
    {
        k::endpoint e{ "0.0.0.0", "http" };
        std::istringstream in{ "[AA:bb::1:http" };
        in >> e;
        ASSERT_TRUE( in.fail() );
        ASSERT_EQ( "0.0.0.0", e.address() );
        ASSERT_EQ( "http", e.service() );
    }
}
