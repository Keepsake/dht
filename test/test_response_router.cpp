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

#include <gtest/gtest.h>

#include <boost/asio/io_service.hpp>

#include "common.hpp"

#include "response_router.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

TEST( response_router_test_construction, can_be_constructed_using_a_reactor )
{
    boost::asio::io_service io_service;
    ASSERT_NO_THROW( kd::response_router{ io_service } );
}

class response_router_test_usage : public testing::Test
{
protected:
    response_router_test_usage()
        : io_service_{}
        , router_{ io_service_ }
        , messages_received_count_{}
        , error_count_{}
    { }

    boost::asio::io_service io_service_;
    kd::response_router router_;
    std::size_t messages_received_count_;
    std::size_t error_count_;
};

TEST_F( response_router_test_usage, known_messages_are_forwarded )
{
    // Create the callbacks.
    auto on_message_received = [ this ]
            ( kd::response_callbacks::endpoint_type const& s
            , kd::header const& h
            , kd::buffer::const_iterator
            , kd::buffer::const_iterator )
    { ++ messages_received_count_; };

    auto on_error = [ this ]
        ( std::error_code const& failure )
    { ++ error_count_; };

    kd::header const h1{ kd::header::V1, kd::header::PING_REQUEST
                       , kd::id{}, kd::id{ "1" } };

    router_.register_temporary_callback( h1.random_token_
                                       , std::chrono::hours{ 1 }
                                       , on_message_received
                                       , on_error );

    io_service_.poll();
    ASSERT_EQ( 0ULL, messages_received_count_ );
    ASSERT_EQ( 0ULL, error_count_ );

    kd::response_callbacks::endpoint_type const s{};
    kd::buffer const b;

    // Send the expected message.
    router_.handle_new_response( s, h1, b.begin(), b.end() );

    io_service_.poll();
    ASSERT_EQ( 1ULL, messages_received_count_ );
    ASSERT_EQ( 0ULL, error_count_ );
}

TEST_F( response_router_test_usage, known_messages_are_not_forwarded_when_late )
{
    // Create the callbacks.
    auto on_message_received = [ this ]
            ( kd::response_callbacks::endpoint_type const& s
            , kd::header const& h
            , kd::buffer::const_iterator
            , kd::buffer::const_iterator )
    { ++ messages_received_count_; };

    auto on_error = [ this ]
        ( std::error_code const& failure )
    { ++ error_count_; };

    kd::header const h1{ kd::header::V1, kd::header::PING_REQUEST
                       , kd::id{}, kd::id{ "1" } };

    router_.register_temporary_callback( h1.random_token_
                                       , std::chrono::hours::zero()
                                       , on_message_received
                                       , on_error );

    io_service_.run_one();
    ASSERT_EQ( 0ULL, messages_received_count_ );
    ASSERT_EQ( 1ULL, error_count_ );

    kd::response_callbacks::endpoint_type const s{};
    kd::buffer const b;

    // Send the expected message.
    router_.handle_new_response( s, h1, b.begin(), b.end() );

    io_service_.poll();
    ASSERT_EQ( 0ULL, messages_received_count_ );
    ASSERT_EQ( 1ULL, error_count_ );
}
