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

#include "common.hpp"

#include "message_serializer.hpp"
#include "message.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

class message_serializer_test_message_serializer : public testing::Test
{
protected:
    message_serializer_test_message_serializer
        ( void )
            : id_{ "abcd" }
    {}

    kd::id id_;
};

TEST_F( message_serializer_test_message_serializer, can_be_constructed )
{
    kd::message_serializer s{ id_ };
    (void)s;
}

TEST_F( message_serializer_test_message_serializer, can_serialize_a_message_with_a_body )
{
    kd::message_serializer s{ id_ };
    kd::id const searched_id{ "1234" };
    kd::id const token{ "ABCD" };

    kd::find_peer_request_body const expected{ searched_id };
    auto const b = s.serialize( expected, token );

    auto i = std::begin( b ), e = std::end( b );
    kd::header h;
    ASSERT_FALSE( kd::deserialize( i, e, h ) );
    ASSERT_EQ( kd::header::V1, h.version_ );
    ASSERT_EQ( kd::header::FIND_PEER_REQUEST, h.type_ );
    ASSERT_EQ( id_, h.source_id_ );
    ASSERT_EQ( token, h.random_token_ );

    kd::find_peer_request_body actual;
    ASSERT_FALSE( kd::deserialize( i, e, actual ) );
    ASSERT_TRUE( expected.peer_to_find_id_ == actual.peer_to_find_id_ );

    ASSERT_TRUE( i == e );
}

TEST_F( message_serializer_test_message_serializer, can_print_a_message_without_body )
{
    kd::message_serializer s{ id_ };
    kd::id const searched_id{ "1234" };
    kd::id const token{ "ABCD" };

    auto const b = s.serialize( kd::header::PING_REQUEST, token );

    auto i = std::begin( b ), e = std::end( b );
    kd::header h;
    ASSERT_FALSE( kd::deserialize( i, e, h ) );
    ASSERT_EQ( kd::header::V1, h.version_ );
    ASSERT_EQ( kd::header::PING_REQUEST, h.type_ );
    ASSERT_EQ( id_, h.source_id_ );
    ASSERT_EQ( token, h.random_token_ );

    ASSERT_TRUE( i == e );
}
