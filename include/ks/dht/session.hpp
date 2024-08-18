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

#pragma once

#include <memory>
#include <system_error>

#include <ks/dht/detail/symbol_visibility.hpp>
#include <ks/dht/endpoint.hpp>
#include <ks/dht/session_base.hpp>

namespace ks::dht {
inline namespace abiv1 {

class session final : public session_base
{
public:
  KS_DHT_EXPORT
  session(endpoint const& initial_peer,
          endpoint const& listen_on_ipv4 = endpoint{ "0.0.0.0", DEFAULT_PORT },
          endpoint const& listen_on_ipv6 = endpoint{ "::", DEFAULT_PORT });

  KS_DHT_EXPORT
  ~session(void);

  session(session const&) = delete;

  session& operator=(session const&) = delete;

  KS_DHT_EXPORT
  void async_save(key_type const& key,
                  data_type const& data,
                  save_handler_type handler);

  template<typename KeyType, typename DataType>
  void async_save(KeyType const& key,
                  DataType const& data,
                  save_handler_type handler)
  {
    async_save(key_type{ std::begin(key), std::end(key) },
               data_type{ std::begin(data), std::end(data) },
               std::move(handler));
  }

  KS_DHT_EXPORT
  void async_load(key_type const& key, load_handler_type handler);

  template<typename KeyType>
  void async_load(KeyType const& key, load_handler_type handler)
  {
    async_load(key_type{ std::begin(key), std::end(key) }, std::move(handler));
  }

  KS_DHT_EXPORT
  std::error_code run(void);

  KS_DHT_EXPORT
  void abort(void);

private:
  struct impl;

private:
  std::unique_ptr<impl> impl_;
};

} // namespace abiv1
} // namespace ks::dht
