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

#include <vector>
#include "common.hpp"

#include "error_impl.hpp"
#include "response_callbacks.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

TEST( response_callbacks_test_construction, can_be_constructed_using_a_reactor )
{
    ASSERT_NO_THROW( kd::response_callbacks{} );
}

class response_callbacks_test_usage : public testing::Test
{
protected:
    response_callbacks_test_usage()
        : callbacks_{}
        , messages_received_{}
    { }

    kd::response_callbacks callbacks_;
    std::vector< kd::id > messages_received_;
};

TEST_F( response_callbacks_test_usage, unknown_message_are_dropped )
{
    kd::response_callbacks::endpoint_type const s{};
    kd::header const h{ kd::header::V1, kd::header::PING_REQUEST };
    kd::buffer const b;
    auto result = callbacks_.dispatch_response( s, h, b.begin(), b.end() );
    ASSERT_EQ( k::UNASSOCIATED_MESSAGE_ID, result );
}

TEST_F( response_callbacks_test_usage, known_messages_are_forwarded )
{
    kd::header const h1{ kd::header::V1, kd::header::PING_REQUEST
                       , kd::id{}, kd::id{ "1" } };
    kd::header const h2{ kd::header::V1, kd::header::PING_REQUEST };
    kd::buffer const b;

    ASSERT_EQ( 0, messages_received_.size() );

    // Create the callback.
    auto on_message_received = [ this ]
            ( kd::response_callbacks::endpoint_type const& s
            , kd::header const& h
            , kd::buffer::const_iterator
            , kd::buffer::const_iterator )
    { messages_received_.push_back( h.random_token_ ); };
    callbacks_.push_callback( h1.random_token_
                                , on_message_received );
    ASSERT_EQ( 0, messages_received_.size() );

    kd::response_callbacks::endpoint_type const s{};

    // Send an unexpected message.
    auto result = callbacks_.dispatch_response( s, h2, b.begin(), b.end() );
    ASSERT_EQ( k::UNASSOCIATED_MESSAGE_ID, result );
    ASSERT_EQ( 0, messages_received_.size() );

    // Send the expected message.
    result = callbacks_.dispatch_response( s, h1, b.begin(), b.end() );
    ASSERT_FALSE( result );
    ASSERT_EQ( 1, messages_received_.size() );
    ASSERT_EQ( h1.random_token_, messages_received_.front() );

    // Send the previously expected message again.
    result = callbacks_.dispatch_response( s, h1, b.begin(), b.end() );
    ASSERT_EQ( k::UNASSOCIATED_MESSAGE_ID, result );
    ASSERT_EQ( 1, messages_received_.size() );
}

TEST_F( response_callbacks_test_usage, multiple_callbacks_can_be_added )
{
    kd::header const h1{ kd::header::V1, kd::header::PING_REQUEST
                       , kd::id{}, kd::id{ "1" } };
    kd::header const h2{ kd::header::V1, kd::header::PING_REQUEST
                       , kd::id{}, kd::id{ "2" } };
    kd::buffer const b;

    ASSERT_EQ( 0, messages_received_.size() );
    // Create the callback.
    auto on_message_received = [ this ]
            ( kd::response_callbacks::endpoint_type const& s
            , kd::header const& h
            , kd::buffer::const_iterator
            , kd::buffer::const_iterator )
    {
        messages_received_.push_back( h.random_token_ );
    };
    callbacks_.push_callback( h1.random_token_
                            , on_message_received );
    callbacks_.push_callback( h2.random_token_
                            , on_message_received );

    kd::response_callbacks::endpoint_type const s{};
    auto result = callbacks_.dispatch_response( s, h1, b.begin(), b.end() );
    ASSERT_FALSE( result );
    ASSERT_EQ( 1, messages_received_.size() );
    ASSERT_EQ( h1.random_token_, messages_received_.front() );

    result = callbacks_.dispatch_response( s, h2, b.begin(), b.end() );
    ASSERT_FALSE( result );
    ASSERT_EQ( 2, messages_received_.size() );
    ASSERT_EQ( h2.random_token_, messages_received_.back() );
}
