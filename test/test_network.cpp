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

#include <ks/dht/endpoint.hpp>

#include "common.hpp"
#include "message_socket.hpp"
#include "network.hpp"
#include "socket_mock.hpp"

namespace k = ks::dht;
namespace kd = k::detail;

using socket_type = kd::message_socket< socket_mock >;
using network_type = kd::network< socket_type >;

class network_test_usage : public testing::Test
{
public:
    network_test_usage
        ( void )
            : io_service_()
            , ipv4_( "172.0.0.1", 1234 )
            , ipv6_( "::1", 1234 )
    { }

    void
    on_message_received
        ( network_type::endpoint_type const&
        , kd::buffer::const_iterator
        , kd::buffer::const_iterator )
    { };

    boost::asio::io_service io_service_;
    k::endpoint ipv4_;
    k::endpoint ipv6_;
};

TEST_F( network_test_usage, schedule_receive_on_construction )
{
    network_type{ io_service_
                , socket_type::ipv4( io_service_, ipv4_ )
                , socket_type::ipv6( io_service_, ipv6_ )
                , std::bind_front( &network_test_usage::on_message_received
                                 , this ) };
}
