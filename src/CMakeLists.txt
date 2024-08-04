# Copyright (c) 2013, David Keller
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the University of California, Berkeley nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY DAVID KELLER AND CONTRIBUTORS ``AS IS AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

add_library(ks-dht
  endpoint.cpp
  error.cpp
  error.cpp
  error_impl.cpp
  first_session.cpp
  id.cpp
  ip_endpoint.cpp
  log.cpp
  message.cpp
  message_serializer.cpp
  peer.cpp
  response_callbacks.cpp
  session.cpp
  session_base.cpp
  timer.cpp
)

target_sources(
  ks-dht
  PUBLIC
    FILE_SET headers
      TYPE
        HEADERS
      BASE_DIRS
        ${PROJECT_BINARY_DIR}/include/
        ${PROJECT_SOURCE_DIR}/include/
      FILES
        ${PROJECT_BINARY_DIR}/include/ks/dht/detail/symbol_visibility.hpp
        ${PROJECT_SOURCE_DIR}/include/ks/dht/endpoint.hpp
        ${PROJECT_SOURCE_DIR}/include/ks/dht/error.hpp
        ${PROJECT_SOURCE_DIR}/include/ks/dht/first_session.hpp
        ${PROJECT_SOURCE_DIR}/include/ks/dht/session.hpp
        ${PROJECT_SOURCE_DIR}/include/ks/dht/session_base.hpp
)

set_target_properties(ks-dht
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_link_libraries(ks-dht
  PRIVATE
    OpenSSL::Crypto
    Threads::Threads
    Boost::headers
)

target_include_directories(
  ks-dht
  PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

add_library(KsDht::ks-dht ALIAS
  ks-dht
)

add_library(ks-dht-impl INTERFACE)

target_sources(
  ks-dht-impl
  INTERFACE
    $<TARGET_OBJECTS:ks-dht>
)

target_link_libraries(
  ks-dht-impl
  INTERFACE
    $<TARGET_PROPERTY:ks-dht,LINK_LIBRARIES>
)

target_include_directories(
  ks-dht-impl
  INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    $<TARGET_PROPERTY:ks-dht,INCLUDE_DIRECTORIES>
)

add_library(KsDht::ks-dht-impl ALIAS
  ks-dht-impl
)
