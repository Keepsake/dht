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

#include <iostream>
#include <sstream>

#include <gtest/gtest.h>

#include "common.hpp"
#include "log.hpp"

namespace kd = ks::dht::detail;

struct rdbuf_saver final
{
    rdbuf_saver
        ( std::ostream & stream
        , std::streambuf * buffer )
            : stream_( stream )
            , old_buffer_( stream.rdbuf( buffer ) )
    { }

    ~rdbuf_saver
        ( void )
    { stream_.rdbuf( old_buffer_ ); }

    std::ostream & stream_;
    std::streambuf * old_buffer_;
};

TEST( log_test_usage, can_write_to_debug_log )
{
    std::ostringstream out;

    {
        rdbuf_saver const s{ std::cout, out.rdbuf() };
        auto const ptr = reinterpret_cast< void *>( 0x12345678 );
        kd::get_debug_log( "test", ptr ) << "message" << std::endl;
    }

    ASSERT_EQ( out.str(), "[debug] (test @ 345678) message\n" );
}

TEST( log_test_usage, can_write_to_debug_log_using_macro )
{
    std::ostringstream out;

    {
        rdbuf_saver const s{ std::cout, out.rdbuf() };
        auto const ptr = reinterpret_cast< void *>( 0x12345678 );
        LOG_DEBUG( test, ptr ) << "message" << std::endl;
    }

#ifdef DHT_ENABLE_DEBUG
    ASSERT_TRUE( out.str(), "[debug] (test @ 345678) message\n" );
#else
    ASSERT_TRUE( out.str().empty() );
#endif
}

TEST( log_test_usage, can_enable_log_module )
{
    // By default, unit tests enable log on all modules.
    kd::disable_log_for( "*" );

    ASSERT_FALSE( kd::is_log_enabled( "test1" ) );
    ASSERT_FALSE( kd::is_log_enabled( "test2" ) );

    kd::enable_log_for( "test1" );
    ASSERT_TRUE( kd::is_log_enabled( "test1" ) );
    ASSERT_FALSE( kd::is_log_enabled( "test2" ) );

    kd::enable_log_for( "*" );
    ASSERT_TRUE( kd::is_log_enabled( "test1" ) );
    ASSERT_TRUE( kd::is_log_enabled( "test2" ) );

}

TEST( log_test_usage, can_convert_container_to_string )
{
    {
        std::vector< std::uint8_t > const c{ 'a', 'b', 'c' };
        auto const r = kd::to_string( c );

        ASSERT_EQ( "abc", r );
    }
    {
        std::vector< std::uint8_t > const c{ 1, 2, 3 };
        auto const r = kd::to_string( c );

        ASSERT_EQ( "\\1\\2\\3", r );
    }
}
