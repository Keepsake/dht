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

#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "common.hpp"
#include "peer_factory.hpp"

#include "routing_table.hpp"
#include "ip_endpoint.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

using test_routing_table = kd::routing_table< kd::ip_endpoint >;

TEST( routing_table_test_construction, is_empty_on_construction )
{
    std::default_random_engine random_engine;

    // Create an empty routing_table.
    test_routing_table rt{ kd::id( random_engine ) };
    // Doesn't contain any peer.
    ASSERT_EQ( rt.peer_count(), 0 );
}

TEST( routing_table_test_push, largest_k_bucket_can_receive_unlimited_peers )
{
    // My id is 128 bit assigned to 0.
    kd::id const my_id;
    // Each bucket can contain up to 4 peers.
    std::size_t const bucket_size = 2;

    test_routing_table rt{ my_id, bucket_size };

    // This peer will be associated with every id.
    // Unicity applies only to id, not peer.
    auto const test_peer( create_endpoint() );

    // Theses should go into high bucket.
    ASSERT_TRUE( rt.push( kd::id{ "10" }, test_peer ) );
    ASSERT_TRUE( rt.push( kd::id{ "11" }, test_peer ) );

    // While theses go to a lower bucket.
    ASSERT_TRUE( rt.push( kd::id{ "20" }, test_peer ) );
    ASSERT_TRUE( rt.push( kd::id{ "21" }, test_peer ) );
    // This one should flag the lower bucket as the largest.
    ASSERT_TRUE( rt.push( kd::id{ "22" }, test_peer ) );
    // This one can be saved as the largest bucket
    // is allowed to exceed bucket_size.
    ASSERT_TRUE( rt.push( kd::id{ "23" }, test_peer ) );

    // This one can't go into first bucket as it is full
    // and is not the largest bucket.
    ASSERT_FALSE( rt.push( kd::id{ "12" }, test_peer ) );
}

TEST( routing_table_test_push, discards_already_pushed_ids )
{
    std::default_random_engine random_engine;

    test_routing_table rt{ kd::id{ random_engine } };
    auto const test_peer( create_endpoint() );
    kd::id test_id;

    // Push two times the same peer.
    ASSERT_EQ( rt.push( test_id, test_peer ), true );
    // Expect the second call to fail.
    ASSERT_EQ( rt.push( test_id, test_peer ), false );
    ASSERT_EQ( rt.peer_count(), 1 );
}

TEST( routing_table_test_find, can_find_a_peer )
{
    test_routing_table rt{ kd::id{} };
    auto test_peer( create_endpoint() );
    kd::id test_id{ "a" };
    ASSERT_TRUE( rt.push( test_id, test_peer ) );

    // Try to find the generated peer.
    auto i = rt.find( test_id );
    ASSERT_NE( i, rt.end() );
    ASSERT_EQ( test_peer, i->second );
}

TEST( routing_table_test_find, can_find_a_closer_peer )
{
    test_routing_table rt{ kd::id{} };

    auto test_peer1( create_endpoint() );
    kd::id test_id1{ "1" };
    ASSERT_TRUE( rt.push( test_id1, test_peer1 ) );

    auto test_peer2( create_endpoint() );
    kd::id test_id2{ "2" };
    ASSERT_TRUE( rt.push( test_id2, test_peer2 ) );

    auto test_peer3( create_endpoint() );
    kd::id test_id3{ "4" };
    ASSERT_TRUE( rt.push( test_id3, test_peer3 ) );

    // test_peer2 is the closest peer.
    auto i = rt.find( kd::id{ "6" } );
    ASSERT_NE( i, rt.end() );
    ASSERT_EQ( test_peer2, i->second );
}

TEST( routing_table_test_find, iterator_start_from_the_closest_k_bucket )
{
    test_routing_table rt( kd::id{}, 1 );
    auto test_peer1( create_endpoint( "192.168.0.1" ) );
    kd::id id1{ "1" };
    ASSERT_TRUE( rt.push( id1, test_peer1 ) );

    auto test_peer2( create_endpoint( "192.168.0.2" ) );
    kd::id id2{ "2" };
    ASSERT_TRUE( rt.push( id2, test_peer2 ) );

    auto test_peer3( create_endpoint( "192.168.0.3" ) );
    kd::id id3{ "4" };
    ASSERT_TRUE( rt.push( id3, test_peer3 ) );

    // Ask for id of the closest peer, and expect to see
    // all of them.
    auto i = rt.find( id1 );

    // This one should be in the close bucket.
    ASSERT_NE( i, rt.end() );
    ASSERT_EQ( test_peer1, i->second );
    ++ i;

    // This one too.
    ASSERT_NE( i, rt.end() );
    ASSERT_EQ( test_peer2, i->second );
    ++ i;

    // This one in the far bucket.
    ASSERT_NE( i, rt.end() );
    ASSERT_EQ( test_peer3, i->second );
    ++ i;

    ASSERT_EQ( i, rt.end() );
}

TEST( routing_table_test_find, iterator_skip_empty_k_bucket )
{
    test_routing_table rt{ kd::id{}, 1 };
    // Fill far k_bucket.
    auto test_peer1( create_endpoint( "192.168.0.1" ) );
    kd::id id1{ "1" };
    ASSERT_TRUE( rt.push( id1, test_peer1 ) );

    // Skip the next "2".

    // End with this one.
    auto test_peer2( create_endpoint( "192.168.0.2" ) );
    kd::id id2{ "4" };
    ASSERT_TRUE( rt.push( id2, test_peer2 ) );

    // Ask for id of the closest peer, and expect to see
    // all of them.
    auto i = rt.find( id1 );

    ASSERT_NE( i, rt.end() );
    ASSERT_EQ( test_peer1, i->second );
    ++ i;

    ASSERT_NE( i, rt.end() );
    ASSERT_EQ( test_peer2, i->second );
    ++ i;

    ASSERT_EQ( i, rt.end() );
}

TEST( routing_table_test_remove, can_remove_a_peer )
{
    test_routing_table rt{ kd::id{} };
    auto test_peer( create_endpoint() );
    kd::id test_id{};
    ASSERT_TRUE( rt.push( test_id, test_peer ) );

    // Try to find the generated peer.
    ASSERT_TRUE( rt.find( test_id ) != rt.end() );
    std::size_t saved_table_size = rt.peer_count();
    ASSERT_EQ( rt.remove( test_id ), true );
    ASSERT_EQ( rt.peer_count(), saved_table_size - 1 );
    ASSERT_TRUE( rt.find( test_id ) == rt.end() );
}

TEST( routing_table_test_print, print_empty_test_routing_table )
{
    std::ostringstream expected;
    expected << std::ifstream{ get_capture_path( "pattern_empty_routing_table.out" ) }.rdbuf();

    std::ostringstream out;
    out << test_routing_table{ kd::id{}, 20 };

    ASSERT_EQ( out.str(), expected.str() );
}
