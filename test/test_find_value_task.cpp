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

#include <vector>
#include <utility>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "common.hpp"
#include "task_fixture.hpp"

#include "id.hpp"
#include "peer.hpp"
#include "ip_endpoint.hpp"
#include "find_value_task.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

using data_type = std::vector< std::uint8_t >;

struct find_value_task_test_usage : task_fixture
{
    find_value_task_test_usage 
        ( void )
        : task_fixture()
        , data_()
    { }

    void
    operator()
        ( std::error_code const& f
        , data_type const& d )
    {
        ++ callback_call_count_;
        failure_ = f;
        data_ = d;
    }

    data_type data_;
};

TEST_F( find_value_task_test_usage, can_notify_error_when_routing_table_is_empty )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    ASSERT_EQ( 0, routing_table_.find_call_count_ );

    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );

    io_service_.poll();

    // Task queried routing table to find closest known peers.
    ASSERT_EQ( 1, routing_table_.find_call_count_ );

    // Task didn't send any more message.
    ASSERT_FALSE( tracker_.has_sent_message() );

    // Task notified the error.
    ASSERT_EQ( 1, callback_call_count_ );
    ASSERT_EQ( failure_, k::VALUE_NOT_FOUND );
}

TEST_F( find_value_task_test_usage, can_notify_error_when_unique_peer_fails_to_respond )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = create_and_add_peer( "192.168.1.1", kd::id{ "a" } );

    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    ASSERT_EQ( 1, routing_table_.find_call_count_ );

    // Task asked p1 for a closer peer or the value.
    kd::find_value_request_body const fv{ searched_key };
    ASSERT_TRUE( tracker_.has_sent_message( p1.endpoint_, fv ) );

    // Task didn't send any more message.
    ASSERT_FALSE( tracker_.has_sent_message() );

    // Task notified the error.
    ASSERT_EQ( 1, callback_call_count_ );
    ASSERT_EQ( failure_, k::VALUE_NOT_FOUND );
}

TEST_F( find_value_task_test_usage, can_notify_error_when_all_peers_fail_to_respond )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = create_and_add_peer( "192.168.1.1", kd::id{ "b" } );
    auto p2 = create_and_add_peer( "192.168.1.2", kd::id{ "c" } );

    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    ASSERT_EQ( 1, routing_table_.find_call_count_ );

    // Task asked p1 & p2 for a closer peer or the value.
    kd::find_value_request_body const fv{ searched_key };
    ASSERT_TRUE( tracker_.has_sent_message( p1.endpoint_, fv ) );
    ASSERT_TRUE( tracker_.has_sent_message( p2.endpoint_, fv ) );

    // Task didn't send any more message.
    ASSERT_FALSE( tracker_.has_sent_message() );

    // Task notified the error.
    ASSERT_EQ( 1, callback_call_count_ );
    ASSERT_EQ( failure_, k::VALUE_NOT_FOUND );
}

TEST_F( find_value_task_test_usage, can_notify_error_when_no_already_known_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = create_and_add_peer( "192.168.1.1", kd::id{ "b" } );
    auto p2 = create_and_add_peer( "192.168.1.2", kd::id{ "c" } );

    // p1 doesn't know closer peer.
    tracker_.add_message_to_receive( p1.endpoint_
                                   , p1.id_
                                   , kd::find_peer_response_body{} );

    // p2 doesn't know closer peer.
    tracker_.add_message_to_receive( p2.endpoint_
                                   , p2.id_
                                   , kd::find_peer_response_body{} );

    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    ASSERT_EQ( 1, routing_table_.find_call_count_ );

    // Task asked p1 & p2 for a closer peer or the value.
    kd::find_value_request_body const fv{ searched_key };
    ASSERT_TRUE( tracker_.has_sent_message( p1.endpoint_, fv ) );
    ASSERT_TRUE( tracker_.has_sent_message( p2.endpoint_, fv ) );

    // Task didn't send any more message.
    ASSERT_FALSE( tracker_.has_sent_message() );

    // Task notified the error.
    ASSERT_EQ( 1, callback_call_count_ );
    ASSERT_EQ( failure_, k::VALUE_NOT_FOUND );
}

TEST_F( find_value_task_test_usage, can_notify_error_when_no_discovered_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    // p1 is the only known peer atm.
    auto p1 = create_and_add_peer( "192.168.1.1", kd::id{ "b" } );

    // p2 is unknown atm.
    auto p2 = create_peer( "192.168.1.2", kd::id{ searched_key } );

    // p1 knows p2.
    kd::find_peer_response_body const fp1{ { p2 } };
    tracker_.add_message_to_receive( p1.endpoint_, p1.id_, fp1 );

    // p2 does'nt known closer peer nor has the value.
    tracker_.add_message_to_receive( p2.endpoint_ 
                                   , p2.id_
                                   , kd::find_peer_response_body{} );

    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    ASSERT_EQ( 1, routing_table_.find_call_count_ );

    // Task asked p1 & p2 for a closer peer or the value.
    kd::find_value_request_body const fv{ searched_key };
    ASSERT_TRUE( tracker_.has_sent_message( p1.endpoint_, fv ) );
    ASSERT_TRUE( tracker_.has_sent_message( p2.endpoint_, fv ) );

    // Task didn't send any more message.
    ASSERT_FALSE( tracker_.has_sent_message() );

    // Task notified the error.
    ASSERT_EQ( 1, callback_call_count_ );
    ASSERT_EQ( failure_, k::VALUE_NOT_FOUND );
}

TEST_F( find_value_task_test_usage, can_return_value_when_already_known_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = create_and_add_peer( "192.168.1.1", kd::id{ "b" } );
    kd::find_value_response_body const b1{ { 1, 2, 3, 4 } };
    tracker_.add_message_to_receive( p1.endpoint_, p1.id_, b1 );
    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    ASSERT_EQ( 1, routing_table_.find_call_count_ );

    // Task asked p1 for a closer peer or the value.
    kd::find_value_request_body const fv{ searched_key };
    ASSERT_TRUE( tracker_.has_sent_message( p1.endpoint_, fv ) );
    ASSERT_FALSE( tracker_.has_sent_message() );

    // Task notified the success.
    ASSERT_EQ( 1, callback_call_count_ );
    ASSERT_FALSE( failure_ );
    ASSERT_THAT( b1.data_, testing::ContainerEq( data_ ) );
}

TEST_F( find_value_task_test_usage, can_return_value_when_discovered_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    // p1 is the only known peer atm.
    auto p1 = create_and_add_peer( "192.168.1.1", kd::id{ "b" } );

    // p2 is unknown atm.
    auto p2 = create_peer( "192.168.1.2", kd::id{ searched_key } );

    // p1 knows p2.
    kd::find_peer_response_body const fp1{ { p2 } };
    tracker_.add_message_to_receive( p1.endpoint_, p1.id_, fp1 );

    // And p2 knows the value.
    kd::find_value_response_body const fv2{ { 1, 2, 3, 4 } };
    tracker_.add_message_to_receive( p2.endpoint_, p2.id_, fv2 );
    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    ASSERT_EQ( 1, routing_table_.find_call_count_ );

    // Task asked p1 for a closer peer or the value.
    kd::find_value_request_body const fv{ searched_key };
    ASSERT_TRUE( tracker_.has_sent_message( p1.endpoint_, fv ) );
    ASSERT_TRUE( tracker_.has_sent_message( p2.endpoint_, fv ) );

    // Task didn't send any more message.
    ASSERT_FALSE( tracker_.has_sent_message() );

    // Task notified the success.
    ASSERT_EQ( 1, callback_call_count_ );
    ASSERT_FALSE( failure_ );
    ASSERT_THAT( fv2.data_, testing::ContainerEq( data_ ) );
}
