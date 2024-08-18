# SPDX-License-Identifier: MIT

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
    Boost::headers
    KsCrypto::ks-crypto
    KsSerialization::ks-serialization
    Threads::Threads
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
