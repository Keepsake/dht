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

#include <cstdint>
#include <future>

#include <gtest/gtest.h>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/system/system_error.hpp>
#include <ks/dht/error.hpp>
#include <ks/dht/first_session.hpp>

#include "common.hpp"
#include "network.hpp"

namespace k = ks::dht;
namespace bo = boost::asio;

TEST( first_session_test_construction, first_session_opens_sockets_on_all_interfaces_by_default )
{
    k::first_session s;

    check_listening( "0.0.0.0", k::first_session::DEFAULT_PORT );
    check_listening( "::", k::first_session::DEFAULT_PORT );
}

TEST( first_session_test_construction, first_session_opens_both_ipv4_ipv6_sockets )
{
    // Create listening socket.
    std::uint16_t const port1 = get_temporary_listening_port();
    std::uint16_t const port2 = get_temporary_listening_port( port1 );
    k::endpoint ipv4_endpoint{ "127.0.0.1", port1 };
    k::endpoint ipv6_endpoint{ "::1", port2 };

    k::first_session s{ ipv4_endpoint, ipv6_endpoint };

    check_listening( "127.0.0.1", port1 );
    check_listening( "::1", port2 );
}

TEST( first_session_test_construction, first_session_throw_on_invalid_ipv6_address )
{
    // Create listening socket.
    std::uint16_t const port1 = get_temporary_listening_port();
    std::uint16_t const port2 = get_temporary_listening_port( port1 );
    k::endpoint ipv4_endpoint{ "127.0.0.1", port1 };
    k::endpoint ipv6_endpoint{ "0.0.0.0", port2 };

    ASSERT_THROW( k::first_session s( ipv4_endpoint
                                           , ipv6_endpoint )
                       , std::exception );
}

TEST( first_session_test_construction, first_session_throw_on_invalid_ipv4_address )
{
    // Create listening socket.
    std::uint16_t const port1 = get_temporary_listening_port();
    std::uint16_t const port2 = get_temporary_listening_port( port1 );
    k::endpoint ipv4_endpoint{ "::", port1 };
    k::endpoint ipv6_endpoint{ "::1", port2 };

    ASSERT_THROW( k::first_session s( ipv4_endpoint
                                           , ipv6_endpoint )
                       , std::exception );
}

TEST( first_session_test_usage, first_session_run_can_be_aborted )
{
    k::first_session s;

    auto result = std::async( std::launch::async
                            , &k::first_session::run, &s );
    s.abort();

    ASSERT_TRUE( result.get() == k::RUN_ABORTED );
}
