# SPDX-License-Identifier: MIT

add_library(ks-dht
  error.cpp
  id.cpp
)

set_target_properties(ks-dht
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
)

target_link_libraries(ks-dht
  PUBLIC
    asio::asio
    KsSerialization::ks-serialization
  PRIVATE
    KsCrypto::ks-crypto
    Threads::Threads
)

add_library(KsDht::ks-dht ALIAS
  ks-dht
)

target_compile_features(ks-dht
  PUBLIC
    cxx_std_23
)
